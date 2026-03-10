#pragma once

#include <string>
#include <unordered_map>

namespace engine {

class Player;

enum class ActionType {
    PlayCard,  // play a card from hand
    EndPhase,  // pass / end the current phase
    Attack,    // declare an attack
    Custom     // game-defined action
};

struct Action {
    ActionType type;
    Player*    actor      = nullptr;

    // PlayCard
    size_t     handIndex  = 0;

    // Attack
    std::string targetId;

    // Custom
    std::string customName;
    std::unordered_map<std::string, std::string> params;
};

struct ValidationResult {
    bool        valid  = true;
    std::string reason;

    static ValidationResult ok()                      { return { true,  {} }; }
    static ValidationResult fail(std::string reason)  { return { false, std::move(reason) }; }

    explicit operator bool() const { return valid; }
};

} // namespace engine
