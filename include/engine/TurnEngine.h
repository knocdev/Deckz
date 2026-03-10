#pragma once

#include "Phase.h"
#include "Player.h"
#include <vector>
#include <functional>

namespace engine {

// Fired when a phase begins or ends
struct PhaseEvent {
    const Phase&  phase;
    Player&       player;
    int           turnNumber;
};

class TurnEngine {
public:
    // phases    — ordered list loaded from Registry
    // players   — non-owning pointers; caller manages lifetime
    TurnEngine(std::vector<Phase> phases, std::vector<Player*> players);

    // --- State ---
    const Phase& currentPhase()  const;
    Player&      currentPlayer() const;
    int          turnNumber()    const { return m_turnNumber; }
    bool         isGameOver()    const { return m_gameOver; }

    // --- Control ---

    // Run the current phase (auto-draw for Draw phases, etc.)
    void executeCurrentPhase();

    // Move to the next phase; rotates players and increments turn when needed
    void advancePhase();

    // Convenience: execute then advance
    void step() { executeCurrentPhase(); advancePhase(); }

    // Signal that the game has ended
    void endGame() { m_gameOver = true; }

    // --- Callbacks (all optional) ---
    void onPhaseStart(std::function<void(const PhaseEvent&)> cb) { m_onPhaseStart = std::move(cb); }
    void onPhaseEnd  (std::function<void(const PhaseEvent&)> cb) { m_onPhaseEnd   = std::move(cb); }
    void onTurnStart (std::function<void(Player&, int)>      cb) { m_onTurnStart  = std::move(cb); }
    void onTurnEnd   (std::function<void(Player&, int)>      cb) { m_onTurnEnd    = std::move(cb); }

private:
    std::vector<Phase>        m_phases;
    std::vector<Player*>      m_players;

    size_t m_phaseIndex  = 0;
    size_t m_playerIndex = 0;
    int    m_turnNumber  = 1;
    bool   m_gameOver    = false;

    std::function<void(const PhaseEvent&)> m_onPhaseStart;
    std::function<void(const PhaseEvent&)> m_onPhaseEnd;
    std::function<void(Player&, int)>      m_onTurnStart;
    std::function<void(Player&, int)>      m_onTurnEnd;

    void firePhaseStart();
    void firePhaseEnd();
};

} // namespace engine
