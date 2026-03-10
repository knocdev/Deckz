#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/Player.h"

using namespace engine;

namespace {
    std::unique_ptr<Deck> emptyDeck() {
        DeckType dt; dt.name = "Test";
        return std::make_unique<Deck>("deck", dt);
    }

    CardType creatureType() {
        CardType ct("Creature");
        ct.addAttribute("attack", AttributeType::Int, 0);
        return ct;
    }
}

TEST(PlayerTest, NameAndEmptyDeck) {
    auto p = Player("Alice", emptyDeck());
    EXPECT_EQ(p.name(), "Alice");
    EXPECT_TRUE(p.deck().empty());
    EXPECT_TRUE(p.hand().empty());
}

TEST(PlayerTest, SetAndGetResource) {
    auto p = Player("Bob", emptyDeck());
    p.setResource("health", 20);
    EXPECT_EQ(p.getResource("health"), 20);
}

TEST(PlayerTest, MissingResourceThrows) {
    auto p = Player("Bob", emptyDeck());
    EXPECT_THROW(p.getResource("mana"), std::out_of_range);
}

TEST(PlayerTest, HasResource) {
    auto p = Player("Bob", emptyDeck());
    EXPECT_FALSE(p.hasResource("health"));
    p.setResource("health", 30);
    EXPECT_TRUE(p.hasResource("health"));
}

TEST(PlayerTest, OverwriteResource) {
    auto p = Player("Bob", emptyDeck());
    p.setResource("health", 20);
    p.setResource("health", 10);
    EXPECT_EQ(p.getResource("health"), 10);
}
