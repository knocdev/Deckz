#pragma once

#include <string>
#include <stdexcept>

namespace engine {

enum class PhaseType {
    Draw,    // automatic card draw
    Action,  // player actions
    Combat,  // attack / defend
    End,     // cleanup
    Custom   // game-defined behaviour
};

inline PhaseType phaseTypeFromString(const std::string& s) {
    if (s == "draw")   return PhaseType::Draw;
    if (s == "action") return PhaseType::Action;
    if (s == "combat") return PhaseType::Combat;
    if (s == "end")    return PhaseType::End;
    if (s == "custom") return PhaseType::Custom;
    throw std::invalid_argument("Unknown phase type: " + s);
}

struct Phase {
    std::string name;
    PhaseType   type;
    int         drawCount       = 1;    // only used when type == Draw
    bool        skipIfHandEmpty = false; // auto-skip this phase if player's hand is empty
};

} // namespace engine
