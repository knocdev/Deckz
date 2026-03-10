#pragma once

#include "Player.h"
#include "TurnEngine.h"
#include <vector>
#include <memory>
#include <functional>
#include <string>

namespace engine {

enum class GameStatus { InProgress, Won, Draw };

// Returned by a win condition check
struct WinCheckResult {
    bool    gameOver = false;
    Player* winner   = nullptr; // nullptr + gameOver = draw
};

class GameState {
public:
    // WinCondition inspects the state and returns a result each time it's checked
    using WinCondition = std::function<WinCheckResult(const GameState&)>;

    // Takes ownership of players and a copy of phases
    GameState(std::vector<std::unique_ptr<Player>> players,
              std::vector<Phase> phases);

    // --- State queries ---
    GameStatus     status()  const { return m_status; }
    bool           isOver()  const { return m_status != GameStatus::InProgress; }
    Player*        winner()  const { return m_winner; }  // nullptr if draw or in progress

    const std::vector<std::unique_ptr<Player>>& players() const { return m_players; }
    Player* findPlayer(const std::string& name) const;

    TurnEngine&       turnEngine()       { return *m_turnEngine; }
    const TurnEngine& turnEngine() const { return *m_turnEngine; }

    // --- Win conditions ---
    void addWinCondition(WinCondition condition);

    // Manually trigger a win condition check (call after card effects, etc.)
    void checkWinConditions();

    // --- Game loop ---
    // Execute current phase → check win conditions → advance phase
    void step();

    // Run until the game is over (use carefully — needs win conditions set)
    void runUntilOver();

private:
    std::vector<std::unique_ptr<Player>> m_players;
    std::unique_ptr<TurnEngine>          m_turnEngine;
    std::vector<WinCondition>            m_winConditions;

    GameStatus m_status = GameStatus::InProgress;
    Player*    m_winner = nullptr;
};

} // namespace engine
