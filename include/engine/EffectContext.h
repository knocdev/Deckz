#pragma once

#include "Player.h"

namespace engine {

class GameState;

// Passed to every effect when it fires
struct EffectContext {
    GameState& state;
    Player*    source;       // player who triggered the effect
    Card*      card;         // card involved (may be null)
    Player*    opponent;     // convenience: first non-source player (may be null)
    size_t     handIndex = 0; // index of the card in source's hand (for remove_from_hand)
};

} // namespace engine
