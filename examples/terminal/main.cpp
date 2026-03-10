#include "engine/Game.h"
#include "ui/TerminalUI.h"
#include <iostream>

int main() {
    try {
        engine::Game game("examples/basic/config.json");

        // --- Validation rules ---
        game.validator().addGlobalRule(engine::Rules::actorNotNull());
        game.validator().addGlobalRule(engine::Rules::isCurrentPlayer());
        game.validator().addRule(engine::ActionType::PlayCard,
                                 engine::Rules::phaseIs(engine::PhaseType::Action));
        game.validator().addRule(engine::ActionType::PlayCard,
                                 engine::Rules::handIndexInRange());
        game.validator().addRule(engine::ActionType::PlayCard,
                                 engine::Rules::hasEnoughResource("mana", "cost"));

        // --- Win condition: first player to reach 0 HP loses ---
        game.addWinCondition([](const engine::GameState& state) -> engine::WinCheckResult {
            for (const auto& p : state.players()) {
                if (p->hasResource("health") && p->getResource("health") <= 0) {
                    for (const auto& other : state.players())
                        if (other.get() != p.get())
                            return { true, other.get() };
                    return { true, nullptr };
                }
            }
            return {};
        });

        // --- Win condition: all players out of cards → draw ---
        game.addWinCondition([](const engine::GameState& state) -> engine::WinCheckResult {
            for (const auto& p : state.players())
                if (!p->deck().empty() || !p->hand().empty())
                    return {};
            return { true, nullptr }; // draw
        });

        // --- Players ---
        // 10 Fireballs each (cost 3, deal 5 damage)
        std::vector<std::string> aliceDeck(10, "fireball");
        std::vector<std::string> bobDeck(10, "fireball");

        game.addPlayer("Alice", "StandardDeck", aliceDeck);
        game.addPlayer("Bob",   "StandardDeck", bobDeck);

        game.start();

        // --- Starting resources ---
        auto* alice = game.state().findPlayer("Alice");
        auto* bob   = game.state().findPlayer("Bob");

        alice->setResource("health", 20);
        alice->setResource("mana",   10);
        bob->setResource("health",   20);
        bob->setResource("mana",     10);

        // --- Run ---
        ui::TerminalUI terminal(game);
        terminal.run();

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
