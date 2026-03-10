#include <gtest/gtest.h>
#include "engine/Registry.h"

using namespace engine;

// Path is relative to where the test binary is run (project root)
static const std::string CONFIG = "examples/basic/config.json";

// ---------------------------------------------------------------------------
// Registry tests
// ---------------------------------------------------------------------------

TEST(RegistryTest, LoadsCardTypes) {
    Registry reg;
    reg.loadFromFile(CONFIG);

    EXPECT_TRUE(reg.hasCardType("Creature"));
    EXPECT_TRUE(reg.hasCardType("Spell"));
}

TEST(RegistryTest, LoadsDeckTypes) {
    Registry reg;
    reg.loadFromFile(CONFIG);

    EXPECT_TRUE(reg.hasDeckType("StandardDeck"));
}

TEST(RegistryTest, LoadsPhases) {
    Registry reg;
    reg.loadFromFile(CONFIG);

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
    reg.loadFromFile(CONFIG);

    const auto& ct = reg.cardType("Creature");
    EXPECT_TRUE(ct.hasAttribute("attack"));
    EXPECT_TRUE(ct.hasAttribute("defense"));
    EXPECT_TRUE(ct.hasAttribute("cost"));
    EXPECT_EQ(ct.getAttribute("attack").type, AttributeType::Int);
}

TEST(RegistryTest, CreateCardWithDefaults) {
    Registry reg;
    reg.loadFromFile(CONFIG);

    auto card = reg.createCard("hero_001", "Creature");
    ASSERT_NE(card, nullptr);
    EXPECT_EQ(card->id(), "hero_001");
    EXPECT_EQ(card->type().name(), "Creature");
    EXPECT_EQ(std::get<int>(card->getAttribute("attack")), 0);
}

TEST(RegistryTest, CreateDeck) {
    Registry reg;
    reg.loadFromFile(CONFIG);

    auto deck = reg.createDeck("PlayerDeck", "StandardDeck");
    ASSERT_NE(deck, nullptr);
    EXPECT_EQ(deck->name(), "PlayerDeck");
    EXPECT_TRUE(deck->empty());
}

TEST(RegistryTest, UnknownCardTypeThrows) {
    Registry reg;
    reg.loadFromFile(CONFIG);
    EXPECT_THROW(reg.cardType("NoSuchType"), std::out_of_range);
}

TEST(RegistryTest, UnknownDeckTypeThrows) {
    Registry reg;
    reg.loadFromFile(CONFIG);
    EXPECT_THROW(reg.deckType("NoSuchDeck"), std::out_of_range);
}

TEST(RegistryTest, MissingFileThrows) {
    Registry reg;
    EXPECT_THROW(reg.loadFromFile("nonexistent.json"), std::runtime_error);
}
