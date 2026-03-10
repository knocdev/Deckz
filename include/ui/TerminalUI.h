#pragma once

#include "engine/Game.h"
#include <string>
#include <vector>

namespace ui {

class TerminalUI {
public:
    explicit TerminalUI(engine::Game& game);

    void run();

private:
    engine::Game&            m_game;
    std::vector<std::string> m_playedThisTurn; // cleared each new turn

    static constexpr int WIDTH     = 80;
    static constexpr int COL_SPLIT = 40; // left panel width

    // --- Rendering ---
    void clearScreen()                              const;
    void render()                                   const;
    void renderPlayerRow()                          const;
    void renderDivider()                            const;
    void renderPlayedCards()                        const;
    void renderTurnInfo()                           const;
    void renderHand(const engine::Player& p)        const;

    // --- Helpers ---
    std::string cardSummary(const engine::Card& c)  const;
    std::string padRight(std::string s, int width)  const;
    std::string padLeft (std::string s, int width)  const;

    // --- Input ---
    std::string prompt(const std::string& text)     const;
    void        message(const std::string& msg)     const;

    // --- Phase handling ---
    void autoPhase();
    void interactivePhase();
    void renderGameOver() const;
};

} // namespace ui
