#include <gtest/gtest.h>
#include "engine/Registry.h"
#include "engine/Game.h"

using namespace engine;

static const std::string CONFIG = R"JSON({
  "card_types": [
    {
      "name": "Creature",
      "attributes": {
        "attack":  { "type": "int", "default": 0 },
        "defense": { "type": "int", "default": 0 },
        "cost":    { "type": "int", "default": 0 }
      }
    },
    {
      "name": "Spell",
      "attributes": {
        "cost": { "type": "int", "default": 0 }
      }
    }
  ],
  "deck_types": [
    { "name": "StandardDeck", "min_cards": 0, "max_cards": 60 }
  ],
  "phases": [
    { "name": "Draw",   "type": "draw", "draw_count": 1 },
    { "name": "Main",   "type": "action" },
    { "name": "Combat", "type": "combat" },
    { "name": "End",    "type": "end" }
  ]
})JSON";

// ---------------------------------------------------------------------------
// Registry tests
// ---------------------------------------------------------------------------

TEST(RegistryTest, LoadsCardTypes) {
    Registry reg;
    reg.loadFromString(CONFIG);

    EXPECT_TRUE(reg.hasCardType("Creature"));
    EXPECT_TRUE(reg.hasCardType("Spell"));
}

TEST(RegistryTest, LoadsDeckTypes) {
    Registry reg;
    reg.loadFromString(CONFIG);

    EXPECT_TRUE(reg.hasDeckType("StandardDeck"));
}

TEST(RegistryTest, LoadsPhases) {
    Registry reg;
    reg.loadFromString(CONFIG);

    const auto& phases = reg.phases();
    ASSERT_EQ(phases.size(), 4u);
    EXPECT_EQ(phases[0].name, "Draw");
    EXPECT_EQ(phases[0].type, PhaseType::Draw);
    EXPECT_EQ(phases[0].drawCount, 1);
    EXPECT_EQ(phases[1].name, "Main");
    EXPECT_EQ(phases[2].name, "Combat");
    EXPECT_EQ(phases[3].name, "End");
}

TEST(RegistryTest, CardTypeHasExpectedAttributes) {
    Registry reg;
    reg.loadFromString(CONFIG);

    const auto& ct = reg.cardType("Creature");
    EXPECT_TRUE(ct.hasAttribute("attack"));
    EXPECT_TRUE(ct.hasAttribute("defense"));
    EXPECT_TRUE(ct.hasAttribute("cost"));
    EXPECT_EQ(ct.getAttribute("attack").type, AttributeType::Int);
}

TEST(RegistryTest, CreateCardWithDefaults) {
    Registry reg;
    reg.loadFromString(CONFIG);

    auto card = reg.createCard("hero_001", "Creature");
    ASSERT_NE(card, nullptr);
    EXPECT_EQ(card->id(), "hero_001");
    EXPECT_EQ(card->type().name(), "Creature");
    EXPECT_EQ(std::get<int>(card->getAttribute("attack")), 0);
}

TEST(RegistryTest, CreateDeck) {
    Registry reg;
    reg.loadFromString(CONFIG);

    auto deck = reg.createDeck("PlayerDeck", "StandardDeck");
    ASSERT_NE(deck, nullptr);
    EXPECT_EQ(deck->name(), "PlayerDeck");
    EXPECT_TRUE(deck->empty());
}

TEST(RegistryTest, UnknownCardTypeThrows) {
    Registry reg;
    reg.loadFromString(CONFIG);
    EXPECT_THROW(reg.cardType("NoSuchType"), std::out_of_range);
}

TEST(RegistryTest, UnknownDeckTypeThrows) {
    Registry reg;
    reg.loadFromString(CONFIG);
    EXPECT_THROW(reg.deckType("NoSuchDeck"), std::out_of_range);
}

TEST(RegistryTest, MissingFileThrows) {
    Registry reg;
    EXPECT_THROW(reg.loadFromFile("nonexistent.json"), std::runtime_error);
}

// ---------------------------------------------------------------------------
// Deck entry (count-based) tests
// ---------------------------------------------------------------------------

static const std::string DECK_ENTRY_CONFIG = R"JSON({
  "card_types": [
    {
      "name": "Spell",
      "attributes": {
        "cost": { "type": "int", "default": 1 }
      }
    }
  ],
  "deck_types": [
    { "name": "TestDeck", "min_cards": 0, "max_cards": 30 }
  ],
  "phases": [
    { "name": "Draw", "type": "draw" },
    { "name": "End",  "type": "end"  }
  ],
  "cards": [
    { "id": "fireball", "type": "Spell", "attributes": { "cost": 3 } },
    { "id": "lightning", "type": "Spell", "attributes": { "cost": 2 } }
  ],
  "players": [
    {
      "name": "Alice",
      "deck_type": "TestDeck",
      "deck": [
        { "card": "fireball",  "count": 3 },
        { "card": "lightning", "count": 2 }
      ],
      "starting_resources": { "mana": 5 }
    }
  ],
  "win_conditions": []
})JSON";

TEST(RegistryDeckEntryTest, LoadsPlayerDeckEntries) {
    Registry reg;
    reg.loadFromString(DECK_ENTRY_CONFIG);

    const auto& players = reg.players();
    ASSERT_EQ(players.size(), 1u);
    EXPECT_EQ(players[0].name, "Alice");

    const auto& entries = players[0].deckEntries;
    ASSERT_EQ(entries.size(), 2u);
    EXPECT_EQ(entries[0].cardId, "fireball");
    EXPECT_EQ(entries[0].count,  3);
    EXPECT_EQ(entries[1].cardId, "lightning");
    EXPECT_EQ(entries[1].count,  2);
}

TEST(RegistryDeckEntryTest, DeckCountDefaultsToOne) {
    Registry reg;
    reg.loadFromString(R"JSON({
      "card_types": [ { "name": "Spell", "attributes": { "cost": { "type": "int" } } } ],
      "deck_types": [ { "name": "D", "min_cards": 0 } ],
      "phases":     [ { "name": "End", "type": "end" } ],
      "cards":      [ { "id": "bolt", "type": "Spell" } ],
      "players": [
        { "name": "Bob", "deck_type": "D", "deck": [ { "card": "bolt" } ] }
      ]
    })JSON");

    const auto& entries = reg.players()[0].deckEntries;
    ASSERT_EQ(entries.size(), 1u);
    EXPECT_EQ(entries[0].count, 1);
}

TEST(RegistryDeckEntryTest, GameExpandsDeckCountsCorrectly) {
    Game game;
    game.loadFromString(DECK_ENTRY_CONFIG);
    game.start();

    Player* alice = game.state().findPlayer("Alice");
    ASSERT_NE(alice, nullptr);
    // 3x fireball + 2x lightning = 5 cards total in deck (draw phase hasn't fired yet)
    EXPECT_EQ(alice->deck().size(), 5u);
}

TEST(RegistryDeckEntryTest, CardAttributesPreservedAcrossCopies) {
    Game game;
    game.loadFromString(DECK_ENTRY_CONFIG);
    game.start();

    Player* alice = game.state().findPlayer("Alice");
    ASSERT_NE(alice, nullptr);

    // Draw all cards into hand to inspect them
    while (!alice->deck().empty())
        alice->hand().addCard(alice->deck().drawTop());

    int fireballCount = 0, lightningCount = 0;
    for (const auto& card : alice->hand().cards()) {
        int cost = std::get<int>(card->getAttribute("cost"));
        if (cost == 3) fireballCount++;
        if (cost == 2) lightningCount++;
    }
    EXPECT_EQ(fireballCount, 3);
    EXPECT_EQ(lightningCount, 2);
}
