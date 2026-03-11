#include "engine/GameState.h"

namespace engine {

GameState::GameState(std::vector<std::unique_ptr<Player>> players,
                     std::vector<Phase> phases)
    : m_players(std::move(players))
{
    std::vector<Player*> ptrs;
    ptrs.reserve(m_players.size());
    for (auto& p : m_players)
        ptrs.push_back(p.get());

    m_turnEngine = std::make_unique<TurnEngine>(std::move(phases), std::move(ptrs));
}

Player* GameState::findPlayer(const std::string& name) const {
    for (const auto& p : m_players)
        if (p->name() == name) return p.get();
    return nullptr;
}

void GameState::addWinCondition(WinCondition condition) {
    m_winConditions.push_back(std::move(condition));
}

void GameState::step() {
    if (isOver()) return;

    const Phase& phase = m_turnEngine->currentPhase();

    if (phase.skipIfHandEmpty && m_turnEngine->currentPlayer().hand().empty()) {
        checkWinConditions();
        if (!isOver()) m_turnEngine->advancePhase();
        return;
    }

    m_turnEngine->executeCurrentPhase();
    checkWinConditions();

    if (!isOver())
        m_turnEngine->advancePhase();
}

void GameState::runUntilOver() {
    while (!isOver())
        step();
}

void GameState::checkWinConditions() {
    for (const auto& condition : m_winConditions) {
        WinCheckResult result = condition(*this);
        if (result.gameOver) {
            m_status = result.winner ? GameStatus::Won : GameStatus::Draw;
            m_winner = result.winner;
            m_turnEngine->endGame();
            return;
        }
    }
}

} // namespace engine
