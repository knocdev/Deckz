#pragma once

#include "Action.h"
#include "Phase.h"
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>

namespace engine {

class GameState;

class ActionValidator {
public:
    using Rule = std::function<ValidationResult(const Action&, const GameState&)>;

    // Register a rule for a specific action type
    void addRule(ActionType type, Rule rule);

    // Register a rule that applies to ALL action types
    void addGlobalRule(Rule rule);

    // Run all matching rules; returns the first failure or ok
    ValidationResult validate(const Action& action, const GameState& state) const;

private:
    std::unordered_map<ActionType, std::vector<Rule>> m_rules;
    std::vector<Rule> m_globalRules;
};

// ---------------------------------------------------------------------------
// Built-in rules — compose these when setting up your game
// ---------------------------------------------------------------------------
namespace Rules {

    // Actor must be the current player
    ActionValidator::Rule isCurrentPlayer();

    // Current phase must be of the given type
    ActionValidator::Rule phaseIs(PhaseType type);

    // Current phase must be one of the given types
    ActionValidator::Rule phaseIsOneOf(std::vector<PhaseType> types);

    // Action must have a non-null actor
    ActionValidator::Rule actorNotNull();

    // handIndex must be within the actor's hand
    ActionValidator::Rule handIndexInRange();

    // Actor must have resourceKey >= the card's attribute value at handIndex
    ActionValidator::Rule hasEnoughResource(const std::string& resourceKey,
                                            const std::string& cardAttributeKey);

} // namespace Rules

} // namespace engine
