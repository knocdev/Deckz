#include "ui/TerminalUI.h"
#include "engine/CardType.h"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace ui {

using namespace engine;

// ---------------------------------------------------------------------------
// Constructor — register turn-start hook to clear the played-cards log
// ---------------------------------------------------------------------------

TerminalUI::TerminalUI(Game& game) : m_game(game) {
    game.state().turnEngine().onTurnStart([this](Player&, int) {
        m_playedThisTurn.clear();
    });
}

// ---------------------------------------------------------------------------
// Public
// ---------------------------------------------------------------------------

void TerminalUI::run() {
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
// Rendering — full redraw each time
// ---------------------------------------------------------------------------

void TerminalUI::clearScreen() const {
    // ANSI: cursor home + clear screen
    std::cout << "\033[H\033[2J" << std::flush;
}

void TerminalUI::render() const {
    clearScreen();
    renderDivider();
    renderPlayerRow();
    renderDivider();
    renderPlayedCards();
    renderDivider();
    renderTurnInfo();
    renderDivider();

    const Player& active = m_game.state().turnEngine().currentPlayer();
    renderHand(active);

    renderDivider();
}

void TerminalUI::renderDivider() const {
    std::cout << "  " << std::string(WIDTH - 2, '-') << "\n";
}

void TerminalUI::renderPlayerRow() const {
    const auto& players = m_game.state().players();
    if (players.empty()) return;

    const Player& active = m_game.state().turnEngine().currentPlayer();

    // Gather up to 2 players
    const Player* p1 = players[0].get();
    const Player* p2 = players.size() > 1 ? players[1].get() : nullptr;

    auto playerLine = [&](const Player* p) -> std::string {
        if (!p) return "";
        bool isTurn = (p == &active);
        std::ostringstream s;
        s << (isTurn ? "* " : "  ") << p->name();
        if (p->hasResource("health")) s << "  HP:" << p->getResource("health");
        if (p->hasResource("mana"))   s << "  Mana:" << p->getResource("mana");
        s << "  Hand:" << p->hand().size() << "  Deck:" << p->deck().size();
        return s.str();
    };

    std::string left  = playerLine(p1);
    std::string right = playerLine(p2);

    // Print left panel + right panel on the same row
    std::cout << padRight(left, COL_SPLIT) << "  " << right << "\n";
}

void TerminalUI::renderPlayedCards() const {
    const std::string header = "Played This Turn";
    std::cout << padLeft("  " + header + "  ", WIDTH / 2 + (int)header.size() / 2) << "\n";

    if (m_playedThisTurn.empty()) {
        std::cout << padLeft("(none)", WIDTH / 2 + 3) << "\n";
    } else {
        for (const auto& entry : m_playedThisTurn) {
            int pad = WIDTH / 2 + (int)entry.size() / 2;
            std::cout << padLeft(entry, pad) << "\n";
        }
    }
}

void TerminalUI::renderTurnInfo() const {
    const TurnEngine& te = m_game.state().turnEngine();
    std::ostringstream s;
    s << "  Turn " << te.turnNumber()
      << "  |  " << te.currentPlayer().name()
      << "'s " << te.currentPhase().name << " Phase";
    std::cout << s.str() << "\n";
}

void TerminalUI::renderHand(const Player& p) const {
    const auto& cards = p.hand().cards();
    if (cards.empty()) {
        std::cout << "  (hand is empty)\n";
        return;
    }
    for (size_t i = 0; i < cards.size(); ++i)
        std::cout << "  [" << i << "] " << cardSummary(*cards[i]) << "\n";
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
        // step() may have advanced to next player; capture draw count before that
        (void)before;
    } else {
        m_game.state().step();
    }
}

void TerminalUI::interactivePhase() {
    render();
    Player& active = m_game.state().turnEngine().currentPlayer();
    std::cout << "  Commands:  play <n>   end\n";

    while (!m_game.isOver()) {
        std::string input = prompt("> ");
        if (input.empty()) continue;

        if (input == "end" || input == "e") {
            m_game.state().step();
            break;
        }

        std::istringstream ss(input);
        std::string cmd; ss >> cmd;

        if (cmd == "play" || cmd == "p") {
            int idx = -1;
            ss >> idx;
            if (idx < 0) { message("Usage: play <index>"); continue; }

            const auto& hand = active.hand().cards();
            std::string played;
            if ((size_t)idx < hand.size())
                played = cardSummary(*hand[idx]);

            Action action{ ActionType::PlayCard, &active, static_cast<size_t>(idx) };
            auto result = m_game.submitAction(action);

            if (!result) {
                message("Invalid: " + result.reason);
                continue;
            }

            if (!played.empty())
                m_playedThisTurn.push_back(active.name() + " played: " + played);

            if (m_game.isOver()) break;

            if (active.hand().size() == 0) {
                m_game.state().step();
                break;
            }

            render();
            std::cout << "  Commands:  play <n>   end\n";
        } else {
            message("Unknown command. Try: play <n>  or  end");
        }
    }
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string TerminalUI::cardSummary(const Card& c) const {
    std::ostringstream s;

    // Name attribute or id
    if (c.hasAttribute("name")) {
        const auto& v = c.getAttribute("name");
        if (std::holds_alternative<std::string>(v))
            s << std::get<std::string>(v);
        else
            s << c.id();
    } else {
        s << c.id();
    }

    s << "  (" << c.type().name() << ")";

    // All int attributes except "name"
    for (const auto& [key, schema] : c.type().schema()) {
        if (key == "name") continue;
        if (schema.type == AttributeType::Int)
            s << "  " << key << ":" << std::get<int>(c.getAttribute(key));
    }

    return s.str();
}

std::string TerminalUI::padRight(std::string s, int width) const {
    if ((int)s.size() < width)
        s += std::string(width - s.size(), ' ');
    return s;
}

std::string TerminalUI::padLeft(std::string s, int width) const {
    if ((int)s.size() < width)
        s = std::string(width - s.size(), ' ') + s;
    return s;
}

std::string TerminalUI::prompt(const std::string& text) const {
    std::cout << "  " << text;
    std::string line;
    std::getline(std::cin, line);
    const std::string ws = " \t\r\n";
    size_t start = line.find_first_not_of(ws);
    size_t end   = line.find_last_not_of(ws);
    return (start == std::string::npos) ? "" : line.substr(start, end - start + 1);
}

void TerminalUI::message(const std::string& msg) const {
    std::cout << "  ! " << msg << "\n";
}

// ---------------------------------------------------------------------------
// Game over
// ---------------------------------------------------------------------------

void TerminalUI::renderGameOver() const {
    clearScreen();
    renderDivider();
    const GameState& gs = m_game.state();
    std::string msg;
    if (gs.status() == GameStatus::Won && gs.winner())
        msg = "  GAME OVER  —  " + gs.winner()->name() + " wins!";
    else if (gs.status() == GameStatus::Draw)
        msg = "  GAME OVER  —  Draw!";
    else
        msg = "  GAME OVER";
    std::cout << msg << "\n";
    renderDivider();
}

} // namespace ui
