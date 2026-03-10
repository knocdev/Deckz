#pragma once

#include "engine/Game.h"
#include <string>

namespace ui {

class TerminalUI {
public:
    explicit TerminalUI(engine::Game& game);

    // Run the game loop until the game ends
    void run();

private:
    engine::Game& m_game;

    // Rendering
    void renderState() const;
    void renderPlayerBar(const engine::Player& p, bool isYou) const;
    void renderHand(const engine::Player& p) const;
    void renderSeparator() const;

    // Phase handling
    void autoPhase();           // Draw / End / Combat — no player input needed
    void interactivePhase();    // Action — prompt the active player

    // Input
    std::string prompt(const std::string& text) const;
    void        printMessage(const std::string& msg) const;

    // Game-over screen
    void renderGameOver() const;
};

} // namespace ui
