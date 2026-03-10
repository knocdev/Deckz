#pragma once

#include "Player.h"

namespace engine {

class GameState;

// Passed to every effect when it fires
struct EffectContext {
    GameState& state;
    Player*    source;    // player who triggered the effect
    Card*      card;      // card being played (may be null)
    Player*    opponent;  // convenience: first non-source player (may be null)
};

} // namespace engine
