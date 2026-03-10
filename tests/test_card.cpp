#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/Card.h"

using namespace engine;

// ---------------------------------------------------------------------------
// CardType tests
// ---------------------------------------------------------------------------

TEST(CardTypeTest, StoresName) {
    CardType ct("Creature");
    EXPECT_EQ(ct.name(), "Creature");
}

TEST(CardTypeTest, AddAndRetrieveAttribute) {
    CardType ct("Creature");
    ct.addAttribute("attack", AttributeType::Int, 0);

    EXPECT_TRUE(ct.hasAttribute("attack"));
    EXPECT_FALSE(ct.hasAttribute("mana"));
    EXPECT_EQ(ct.getAttribute("attack").type, AttributeType::Int);
}

TEST(CardTypeTest, UnknownAttributeThrows) {
    CardType ct("Spell");
    EXPECT_THROW(ct.getAttribute("nonexistent"), std::out_of_range);
}

// ---------------------------------------------------------------------------
// Card tests
// ---------------------------------------------------------------------------

TEST(CardTest, DefaultAttributesFromSchema) {
    CardType ct("Creature");
    ct.addAttribute("attack",  AttributeType::Int,    0);
    ct.addAttribute("name",    AttributeType::String, std::string{});

    Card card("c001", ct);
    EXPECT_EQ(std::get<int>(card.getAttribute("attack")), 0);
    EXPECT_EQ(std::get<std::string>(card.getAttribute("name")), "");
}

TEST(CardTest, SetAndGetAttribute) {
    CardType ct("Creature");
    ct.addAttribute("attack", AttributeType::Int, 0);

    Card card("c001", ct);
    card.setAttribute("attack", 5);
    EXPECT_EQ(std::get<int>(card.getAttribute("attack")), 5);
}

TEST(CardTest, SetUnknownAttributeThrows) {
    CardType ct("Creature");
    Card card("c001", ct);
    EXPECT_THROW(card.setAttribute("unknown", 42), std::invalid_argument);
}

TEST(CardTest, GetUnknownAttributeThrows) {
    CardType ct("Creature");
    Card card("c001", ct);
    EXPECT_THROW(card.getAttribute("unknown"), std::out_of_range);
}

TEST(CardTest, CardKnowsItsType) {
    CardType ct("Spell");
    Card card("s001", ct);
    EXPECT_EQ(card.type().name(), "Spell");
    EXPECT_EQ(card.id(), "s001");
}
