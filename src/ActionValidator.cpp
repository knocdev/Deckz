#include "engine/ActionValidator.h"
#include "engine/GameState.h"
#include "engine/CardType.h"

namespace engine {

// ---------------------------------------------------------------------------
// ActionValidator
// ---------------------------------------------------------------------------

void ActionValidator::addRule(ActionType type, Rule rule) {
    m_rules[type].push_back(std::move(rule));
}

void ActionValidator::addGlobalRule(Rule rule) {
    m_globalRules.push_back(std::move(rule));
}

ValidationResult ActionValidator::validate(const Action& action,
                                            const GameState& state) const {
    for (const auto& rule : m_globalRules) {
        auto result = rule(action, state);
        if (!result) return result;
    }

    auto it = m_rules.find(action.type);
    if (it != m_rules.end()) {
        for (const auto& rule : it->second) {
            auto result = rule(action, state);
            if (!result) return result;
        }
    }

    return ValidationResult::ok();
}

// ---------------------------------------------------------------------------
// Built-in rules
// ---------------------------------------------------------------------------

namespace Rules {

ActionValidator::Rule isCurrentPlayer() {
    return [](const Action& action, const GameState& state) -> ValidationResult {
        if (action.actor != &state.turnEngine().currentPlayer())
            return ValidationResult::fail("It is not " +
                (action.actor ? action.actor->name() : "unknown") + "'s turn");
        return ValidationResult::ok();
    };
}

ActionValidator::Rule phaseIs(PhaseType type) {
    return [type](const Action& action, const GameState& state) -> ValidationResult {
        if (state.turnEngine().currentPhase().type != type)
            return ValidationResult::fail("Action not allowed in phase '" +
                state.turnEngine().currentPhase().name + "'");
        return ValidationResult::ok();
    };
}

ActionValidator::Rule phaseIsOneOf(std::vector<PhaseType> types) {
    return [types](const Action& action, const GameState& state) -> ValidationResult {
        const PhaseType current = state.turnEngine().currentPhase().type;
        for (PhaseType t : types)
            if (current == t) return ValidationResult::ok();
        return ValidationResult::fail("Action not allowed in phase '" +
            state.turnEngine().currentPhase().name + "'");
    };
}

ActionValidator::Rule actorNotNull() {
    return [](const Action& action, const GameState&) -> ValidationResult {
        if (!action.actor)
            return ValidationResult::fail("Action has no actor");
        return ValidationResult::ok();
    };
}

ActionValidator::Rule handIndexInRange() {
    return [](const Action& action, const GameState&) -> ValidationResult {
        if (!action.actor)
            return ValidationResult::fail("Action has no actor");
        if (action.handIndex >= action.actor->hand().size())
            return ValidationResult::fail("Hand index " +
                std::to_string(action.handIndex) + " out of range (hand size: " +
                std::to_string(action.actor->hand().size()) + ")");
        return ValidationResult::ok();
    };
}

ActionValidator::Rule hasEnoughResource(const std::string& resourceKey,
                                         const std::string& cardAttributeKey) {
    return [resourceKey, cardAttributeKey](const Action& action,
                                           const GameState&) -> ValidationResult {
        if (!action.actor)
            return ValidationResult::fail("Action has no actor");

        if (!action.actor->hasResource(resourceKey))
            return ValidationResult::fail("Player has no resource '" + resourceKey + "'");

        const auto& hand = action.actor->hand().cards();
        if (action.handIndex >= hand.size())
            return ValidationResult::fail("Hand index out of range");

        const Card& card = *hand[action.handIndex];
        if (!card.hasAttribute(cardAttributeKey))
            return ValidationResult::fail("Card has no attribute '" + cardAttributeKey + "'");

        int cost     = std::get<int>(card.getAttribute(cardAttributeKey));
        int resource = action.actor->getResource(resourceKey);

        if (resource < cost)
            return ValidationResult::fail("Not enough " + resourceKey +
                " (need " + std::to_string(cost) +
                ", have " + std::to_string(resource) + ")");

        return ValidationResult::ok();
    };
}

} // namespace Rules
} // namespace engine
