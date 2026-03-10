#include "engine/Game.h"
#include "ui/TerminalUI.h"
#include <iostream>

int main() {
    try {
        engine::Game game("examples/terminal/config.json");
        game.start();

        ui::TerminalUI terminal(game);
        terminal.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
