#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/Hand.h"

using namespace engine;

namespace {
    CardType spellType() {
        CardType ct("Spell");
        ct.addAttribute("cost", AttributeType::Int, 0);
        return ct;
    }
}

TEST(HandTest, StartsEmpty) {
    Hand h;
    EXPECT_TRUE(h.empty());
    EXPECT_EQ(h.size(), 0u);
}

TEST(HandTest, AddCard) {
    Hand h;
    auto ct = spellType();
    h.addCard(std::make_shared<Card>("s1", ct));
    EXPECT_EQ(h.size(), 1u);
    EXPECT_FALSE(h.empty());
}

TEST(HandTest, RemoveCardByIndex) {
    Hand h;
    auto ct = spellType();
    h.addCard(std::make_shared<Card>("s1", ct));
    h.addCard(std::make_shared<Card>("s2", ct));

    auto removed = h.removeCard(0);
    EXPECT_EQ(removed->id(), "s1");
    EXPECT_EQ(h.size(), 1u);
    EXPECT_EQ(h.cards()[0]->id(), "s2");
}

TEST(HandTest, RemoveOutOfRangeThrows) {
    Hand h;
    EXPECT_THROW(h.removeCard(0), std::out_of_range);
}
