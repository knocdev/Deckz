#include "engine/TurnEngine.h"
#include <stdexcept>

namespace engine {

TurnEngine::TurnEngine(std::vector<Phase> phases, std::vector<Player*> players)
    : m_phases(std::move(phases)), m_players(std::move(players))
{
    if (m_phases.empty())
        throw std::invalid_argument("TurnEngine requires at least one phase");
    if (m_players.empty())
        throw std::invalid_argument("TurnEngine requires at least one player");

    // Fire the first turn/phase start events
    if (m_onTurnStart) m_onTurnStart(*m_players[0], m_turnNumber);
    firePhaseStart();
}

const Phase& TurnEngine::currentPhase() const {
    return m_phases[m_phaseIndex];
}

Player& TurnEngine::currentPlayer() const {
    return *m_players[m_playerIndex];
}

void TurnEngine::executeCurrentPhase() {
    if (m_gameOver) return;

    const Phase& phase = currentPhase();
    Player& player     = currentPlayer();

    if (phase.type == PhaseType::Draw) {
        for (int i = 0; i < phase.drawCount; ++i) {
            if (player.deck().empty()) break;
            player.hand().addCard(player.deck().drawTop());
        }
    }
    // Action / Combat / End / Custom phases are intentionally left to the
    // caller — they hook in via onPhaseStart/onPhaseEnd callbacks.
}

void TurnEngine::advancePhase() {
    if (m_gameOver) return;

    firePhaseEnd();

    m_phaseIndex++;

    // End of this player's turn
    if (m_phaseIndex >= m_phases.size()) {
        if (m_onTurnEnd) m_onTurnEnd(currentPlayer(), m_turnNumber);

        m_phaseIndex = 0;
        m_playerIndex = (m_playerIndex + 1) % m_players.size();

        // All players have gone — new round
        if (m_playerIndex == 0) m_turnNumber++;

        if (m_onTurnStart) m_onTurnStart(currentPlayer(), m_turnNumber);
    }

    firePhaseStart();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void TurnEngine::firePhaseStart() {
    if (m_onPhaseStart)
        m_onPhaseStart({ currentPhase(), currentPlayer(), m_turnNumber });
}

void TurnEngine::firePhaseEnd() {
    if (m_onPhaseEnd)
        m_onPhaseEnd({ currentPhase(), currentPlayer(), m_turnNumber });
}

} // namespace engine
