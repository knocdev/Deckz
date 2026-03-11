#include <gtest/gtest.h>
#include "engine/Registry.h"

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
