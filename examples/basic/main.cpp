#include "engine/Registry.h"
#include <iostream>
#include <sstream>

namespace { std::string itos(int n) { std::ostringstream s; s << n; return s.str(); } }

int main() {
    engine::Registry registry;
    registry.loadFromFile("examples/basic/config.json");

    // --- Inspect loaded phases ---
    std::cout << "Phases:\n";
    for (const auto& phase : registry.phases())
        std::cout << "  " << phase.name << "\n";

    // --- Create some cards ---
    auto dragon = registry.createCard("dragon_001", "Creature");
    dragon->setAttribute("name",    std::string("Fire Dragon"));
    dragon->setAttribute("attack",  5);
    dragon->setAttribute("defense", 3);
    dragon->setAttribute("cost",    4);

    auto fireball = registry.createCard("fireball_001", "Spell");
    fireball->setAttribute("name",   std::string("Fireball"));
    fireball->setAttribute("cost",   2);
    fireball->setAttribute("effect", std::string("Deal 3 damage to any target"));

    // --- Build a deck ---
    auto deck = registry.createDeck("Player1Deck", "StandardDeck");
    for (int i = 0; i < 10; ++i)
        deck->addCard(registry.createCard("dragon_00" + itos(i), "Creature"));

    std::string error;
    if (deck->validate(error))
        std::cout << "\nDeck '" << deck->name() << "' is valid ("
                  << deck->size() << " cards)\n";
    else
        std::cout << "\nDeck invalid: " << error << "\n";

    deck->shuffle();

    auto drawn = deck->drawTop();
    std::cout << "Drew: " << drawn->id()
              << " (type: " << drawn->type().name() << ")\n";

    return 0;
}
