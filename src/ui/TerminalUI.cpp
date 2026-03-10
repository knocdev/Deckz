#include "ui/TerminalUI.h"
#include "engine/CardType.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ui {

using namespace engine;

TerminalUI::TerminalUI(Game& game) : m_game(game) {}

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------

void TerminalUI::run() {
    std::cout << "\n  Card Engine — Terminal\n";
    renderSeparator();

    while (!m_game.isOver()) {
        const Phase& phase = m_game.state().turnEngine().currentPhase();

        if (phase.type == PhaseType::Action || phase.type == PhaseType::Custom)
            interactivePhase();
        else
            autoPhase();
    }

    renderGameOver();
}

// ---------------------------------------------------------------------------
// Rendering
// ---------------------------------------------------------------------------

void TerminalUI::renderState() const {
    const TurnEngine& te      = m_game.state().turnEngine();
    const Player&     active  = te.currentPlayer();

    // Find the first opponent
    const Player* opponent = nullptr;
    for (const auto& p : m_game.state().players())
        if (p.get() != &active) { opponent = p.get(); break; }

    renderSeparator();
    std::cout << "  Turn " << te.turnNumber()
              << "  |  " << active.name() << "'s " << te.currentPhase().name << " Phase\n";
    renderSeparator();

    if (opponent) renderPlayerBar(*opponent, false);

    renderHand(active);
    std::cout << "\n";

    renderPlayerBar(active, true);
    std::cout << "\n";
}

void TerminalUI::renderPlayerBar(const Player& p, bool isYou) const {
    std::cout << "  " << (isYou ? "YOU" : "OPP")
              << "  [" << p.name() << "]";

    if (p.hasResource("health"))
        std::cout << "  HP: " << p.getResource("health");
    if (p.hasResource("mana"))
        std::cout << "  Mana: " << p.getResource("mana");

    std::cout << "  Hand: " << p.hand().size()
              << "  Deck: " << p.deck().size() << "\n";
}

void TerminalUI::renderHand(const Player& p) const {
    const auto& cards = p.hand().cards();
    if (cards.empty()) {
        std::cout << "\n  (hand is empty)\n";
        return;
    }

    std::cout << "\n  Hand:\n";
    for (size_t i = 0; i < cards.size(); ++i) {
        const Card& c = *cards[i];
        std::cout << "    [" << i << "] ";

        // Print name attribute if present, otherwise card id
        if (c.hasAttribute("name")) {
            const auto& v = c.getAttribute("name");
            if (std::holds_alternative<std::string>(v))
                std::cout << std::get<std::string>(v);
            else
                std::cout << c.id();
        } else {
            std::cout << c.id();
        }

        std::cout << "  (" << c.type().name() << ")";

        // Print all int/float attributes except "name"
        for (const auto& [key, schema] : c.type().schema()) {
            if (key == "name") continue;
            if (schema.type == AttributeType::Int) {
                int val = std::get<int>(c.getAttribute(key));
                std::cout << "  " << key << ":" << val;
            }
        }
        std::cout << "\n";
    }
}

void TerminalUI::renderSeparator() const {
    std::cout << "  " << std::string(44, '-') << "\n";
}

// ---------------------------------------------------------------------------
// Phase handling
// ---------------------------------------------------------------------------

void TerminalUI::autoPhase() {
    const Phase&  phase  = m_game.state().turnEngine().currentPhase();
    const Player& active = m_game.state().turnEngine().currentPlayer();

    if (phase.type == PhaseType::Draw) {
        size_t before = active.hand().size();
        m_game.state().step();
        size_t drawn = active.hand().size() - before; // step() may wrap to next player
        if (drawn > 0)
            std::cout << "  >> " << active.name() << " draws " << drawn << " card(s).\n";
    } else {
        m_game.state().step();
    }
}

void TerminalUI::interactivePhase() {
    renderState();

    Player& active = m_game.state().turnEngine().currentPlayer();

    std::cout << "  Commands:  play <n>   end\n";

    while (!m_game.isOver()) {
        std::string input = prompt("> ");
        if (input.empty()) continue;

        if (input == "end" || input == "e") {
            // Advance past Action phase (step() = execute noop + check + advance)
            m_game.state().step();
            break;
        }

        // "play N" or "p N"
        std::istringstream ss(input);
        std::string cmd; ss >> cmd;

        if (cmd == "play" || cmd == "p") {
            int idx = -1;
            ss >> idx;
            if (idx < 0) { printMessage("Usage: play <index>"); continue; }

            Action action{ ActionType::PlayCard, &active, static_cast<size_t>(idx) };
            auto result = m_game.submitAction(action);

            if (!result) {
                printMessage("Invalid: " + result.reason);
            } else {
                if (m_game.isOver()) break;
                if (active.hand().size() == 0) {
                    printMessage("Hand is empty — ending phase.");
                    m_game.state().step();
                    break;
                }
                renderState();
                std::cout << "  Commands:  play <n>   end\n";
            }
        } else {
            printMessage("Unknown command. Try: play <n>  or  end");
        }
    }
}

// ---------------------------------------------------------------------------
// Input / output helpers
// ---------------------------------------------------------------------------

std::string TerminalUI::prompt(const std::string& text) const {
    std::cout << "  " << text;
    std::string line;
    std::getline(std::cin, line);
    // Trim leading/trailing whitespace
    const std::string ws = " \t\r\n";
    size_t start = line.find_first_not_of(ws);
    size_t end   = line.find_last_not_of(ws);
    return (start == std::string::npos) ? "" : line.substr(start, end - start + 1);
}

void TerminalUI::printMessage(const std::string& msg) const {
    std::cout << "  ! " << msg << "\n";
}

void TerminalUI::renderGameOver() const {
    renderSeparator();
    const GameState& gs = m_game.state();

    if (gs.status() == GameStatus::Won && gs.winner())
        std::cout << "  GAME OVER  —  " << gs.winner()->name() << " wins!\n";
    else if (gs.status() == GameStatus::Draw)
        std::cout << "  GAME OVER  —  Draw!\n";
    else
        std::cout << "  GAME OVER\n";

    renderSeparator();
}

} // namespace ui
