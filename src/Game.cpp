#include "engine/Game.h"
#include "engine/EffectContext.h"
#include "engine/CardType.h"
#include <stdexcept>

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
    if (m_playerSetups.empty())
        throw std::runtime_error("Game::start() called with no players");

    std::vector<std::unique_ptr<Player>> players;

    for (const auto& setup : m_playerSetups) {
        auto deck = m_registry.createDeck(setup.name + "_deck", setup.deckTypeName);
        for (const auto& cardId : setup.cardIds)
            deck->addCard(m_registry.createCard(cardId, m_registry.cardIdToTypeName(cardId)));
        players.push_back(std::make_unique<Player>(setup.name, std::move(deck)));
    }

    m_state = std::make_unique<GameState>(std::move(players), m_registry.phases());

    for (auto& cond : m_winConditions)
        m_state->addWinCondition(cond);
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

} // namespace engine
