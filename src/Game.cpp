#include "engine/Game.h"
#include "engine/EffectContext.h"
#include "engine/CardType.h"
#include "engine/TurnEngine.h"
#include "engine/Phase.h"
#include <stdexcept>
#include <limits>

namespace engine {

Game::Game(const std::string& configPath) {
    m_registry.loadFromFile(configPath);
    m_effectRegistry.registerBuiltins();
}

void Game::registerEffect(const std::string& typeName, EffectFactory factory) {
    m_effectRegistry.registerEffect(typeName, std::move(factory));
}

void Game::addWinCondition(GameState::WinCondition condition) {
    m_winConditions.push_back(std::move(condition));
}

void Game::addPlayer(const std::string& playerName,
                     const std::string& deckTypeName,
                     const std::vector<std::string>& cardIds) {
    m_playerSetups.push_back({ playerName, deckTypeName, cardIds });
}

void Game::start() {
    // Merge JSON players with any programmatically added ones
    auto allSetups = m_playerSetups;
    for (const auto& pd : m_registry.players())
        allSetups.push_back({ pd.name, pd.deckTypeName, pd.cardIds });

    if (allSetups.empty())
        throw std::runtime_error("Game::start() called with no players");

    std::vector<std::unique_ptr<Player>> players;

    for (const auto& setup : allSetups) {
        auto deck = m_registry.createDeck(setup.name + "_deck", setup.deckTypeName);
        for (const auto& cardId : setup.cardIds)
            deck->addCard(m_registry.createCard(cardId, m_registry.cardIdToTypeName(cardId)));
        players.push_back(std::make_unique<Player>(setup.name, std::move(deck)));
    }

    m_state = std::make_unique<GameState>(std::move(players), m_registry.phases());

    // Apply starting resources from JSON player definitions
    for (const auto& pd : m_registry.players()) {
        Player* p = m_state->findPlayer(pd.name);
        if (!p) continue;
        for (const auto& [res, val] : pd.startingResources)
            p->setResource(res, val);
    }

    // Programmatic win conditions
    for (auto& cond : m_winConditions)
        m_state->addWinCondition(cond);

    // JSON win conditions
    for (const auto& def : m_registry.winConditions())
        m_state->addWinCondition(buildWinCondition(def));

    // JSON global rules
    for (const auto& def : m_registry.globalRules())
        m_validator.addGlobalRule(buildRule(def));

    // JSON per-action rules — map JSON action name → ActionType
    auto applyActionRules = [&](const std::string& name, ActionType type) {
        for (const auto& def : m_registry.actionRules(name))
            m_validator.addRule(type, buildRule(def));
    };
    applyActionRules("play_card", ActionType::PlayCard);
    applyActionRules("attack",    ActionType::Attack);
    applyActionRules("end_phase", ActionType::EndPhase);

    auto registerTurnEffects = [&](const std::vector<EffectDefinition>& defs,
                                   auto registerFn) {
        if (defs.empty()) return;
        (m_state->turnEngine().*registerFn)([this, defs](Player& p, int) {
            Player* opp = findOpponent(&p);
            EffectContext ctx{ *m_state, &p, nullptr, opp };
            for (const auto& def : defs) {
                if (!m_effectRegistry.has(def.typeName)) continue;
                auto fn = m_effectRegistry.create(def);
                fn(ctx);
            }
        });
    };

    registerTurnEffects(m_registry.turnStartEffects(), &TurnEngine::onTurnStart);
    registerTurnEffects(m_registry.turnEndEffects(),   &TurnEngine::onTurnEnd);
}

ValidationResult Game::submitAction(const Action& action) {
    if (!m_state)
        throw std::runtime_error("Game::submitAction called before start()");

    auto result = m_validator.validate(action, *m_state);
    if (!result) return result;

    if (action.type == ActionType::PlayCard) {
        Action mutableAction = action;
        executePlayCard(mutableAction);
        m_state->checkWinConditions();
    }

    return ValidationResult::ok();
}

GameState& Game::state() {
    if (!m_state) throw std::runtime_error("Game not started");
    return *m_state;
}

const GameState& Game::state() const {
    if (!m_state) throw std::runtime_error("Game not started");
    return *m_state;
}

bool Game::isOver() const {
    return m_state && m_state->isOver();
}

// ---------------------------------------------------------------------------
// Private
// ---------------------------------------------------------------------------

void Game::executePlayCard(Action& action) {
    const auto& hand  = action.actor->hand().cards();
    Card* card        = hand[action.handIndex].get();
    const CardType& ct = card->type();

    // Deduct mana cost if the card has a "cost" attribute and actor has "mana"
    if (ct.hasAttribute("cost") && action.actor->hasResource("mana")) {
        int cost = std::get<int>(card->getAttribute("cost"));
        action.actor->setResource("mana",
            action.actor->getResource("mana") - cost);
    }

    // Trigger OnPlay effects before removing the card (so ctx.card is valid)
    triggerEffects(EffectTrigger::OnPlay, action.actor, card);

    // Remove the card from hand
    action.actor->hand().removeCard(action.handIndex);
}

void Game::triggerEffects(EffectTrigger trigger, Player* actor, Card* card) {
    if (!card) return;

    Player* opp = findOpponent(actor);
    EffectContext ctx{ *m_state, actor, card, opp };

    for (const auto& def : card->type().effects()) {
        if (def.trigger != trigger) continue;
        if (!m_effectRegistry.has(def.typeName)) continue;
        auto fn = m_effectRegistry.create(def);
        fn(ctx);
    }
}

Player* Game::findOpponent(Player* actor) const {
    for (const auto& p : m_state->players())
        if (p.get() != actor) return p.get();
    return nullptr;
}

// ---------------------------------------------------------------------------
// JSON rule / win-condition factories
// ---------------------------------------------------------------------------

ActionValidator::Rule Game::buildRule(const RuleDefinition& def) const {
    if (def.typeName == "actor_not_null")      return Rules::actorNotNull();
    if (def.typeName == "is_current_player")   return Rules::isCurrentPlayer();
    if (def.typeName == "hand_index_in_range") return Rules::handIndexInRange();

    if (def.typeName == "phase_type") {
        std::vector<PhaseType> types;
        std::string allowed = def.params.at("allowed");
        // split comma-joined string
        std::string token;
        for (char c : allowed + ',') {
            if (c == ',') { if (!token.empty()) types.push_back(phaseTypeFromString(token)); token.clear(); }
            else token += c;
        }
        return Rules::phaseIsOneOf(std::move(types));
    }

    if (def.typeName == "has_enough_resource")
        return Rules::hasEnoughResource(def.params.at("resource"),
                                         def.params.at("cost_attribute"));

    throw std::runtime_error("Unknown rule type: " + def.typeName);
}

GameState::WinCondition Game::buildWinCondition(const WinConditionDefinition& def) const {
    if (def.typeName == "resource_depleted") {
        std::string resource = def.params.at("resource");
        std::string result   = def.params.count("result") ? def.params.at("result") : "opponent_wins";
        return [resource, result](const GameState& state) -> WinCheckResult {
            for (const auto& p : state.players()) {
                if (p->hasResource(resource) && p->getResource(resource) <= 0) {
                    if (result == "draw") return { true, nullptr };
                    for (const auto& other : state.players())
                        if (other.get() != p.get())
                            return { true, other.get() };
                    return { true, nullptr };
                }
            }
            return {};
        };
    }

    if (def.typeName == "cards_exhausted") {
        std::string result   = def.params.count("result")   ? def.params.at("result")   : "draw";
        std::string resource = def.params.count("resource") ? def.params.at("resource") : "";
        return [result, resource](const GameState& state) -> WinCheckResult {
            for (const auto& p : state.players())
                if (!p->deck().empty() || !p->hand().empty())
                    return {};
            // All players out of cards — determine outcome
            if (result == "most_resource" && !resource.empty()) {
                Player* winner = nullptr;
                int best = std::numeric_limits<int>::min();
                bool tied = false;
                for (const auto& p : state.players()) {
                    if (!p->hasResource(resource)) continue;
                    int val = p->getResource(resource);
                    if (val > best) { best = val; winner = p.get(); tied = false; }
                    else if (val == best) { tied = true; }
                }
                if (tied) return { true, nullptr };
                return { true, winner };
            }
            return { true, nullptr }; // draw
        };
    }

    throw std::runtime_error("Unknown win condition type: " + def.typeName);
}

} // namespace engine
