#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/Card.h"
#include "engine/Deck.h"

using namespace engine;

namespace {
    CardType makeCreatureType() {
        CardType ct("Creature");
        ct.addAttribute("attack", AttributeType::Int, 0);
        return ct;
    }

    std::shared_ptr<Card> makeCard(const std::string& id, const CardType& ct) {
        return std::make_shared<Card>(id, ct);
    }
}

// ---------------------------------------------------------------------------
// Deck tests
// ---------------------------------------------------------------------------

TEST(DeckTest, StartsEmpty) {
    DeckType dt; dt.name = "Test";
    Deck deck("MyDeck", dt);
    EXPECT_TRUE(deck.empty());
    EXPECT_EQ(deck.size(), 0u);
}

TEST(DeckTest, AddAndSize) {
    DeckType dt; dt.name = "Test";
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    deck.addCard(makeCard("c1", ct));
    deck.addCard(makeCard("c2", ct));
    EXPECT_EQ(deck.size(), 2u);
}

TEST(DeckTest, DrawTopRemovesFromFront) {
    DeckType dt; dt.name = "Test";
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    deck.addCard(makeCard("first", ct));
    deck.addCard(makeCard("second", ct));

    auto drawn = deck.drawTop();
    EXPECT_EQ(drawn->id(), "first");
    EXPECT_EQ(deck.size(), 1u);
}

TEST(DeckTest, DrawBottomRemovesFromBack) {
    DeckType dt; dt.name = "Test";
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    deck.addCard(makeCard("first", ct));
    deck.addCard(makeCard("last", ct));

    auto drawn = deck.drawBottom();
    EXPECT_EQ(drawn->id(), "last");
    EXPECT_EQ(deck.size(), 1u);
}

TEST(DeckTest, DrawFromEmptyThrows) {
    DeckType dt; dt.name = "Test";
    Deck deck("MyDeck", dt);
    EXPECT_THROW(deck.drawTop(),    std::underflow_error);
    EXPECT_THROW(deck.drawBottom(), std::underflow_error);
}

TEST(DeckTest, ValidateMinCards) {
    DeckType dt; dt.name = "Test"; dt.minCards = 5;
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    deck.addCard(makeCard("c1", ct));

    std::string err;
    EXPECT_FALSE(deck.validate(err));
    EXPECT_FALSE(err.empty());
}

TEST(DeckTest, ValidateMaxCards) {
    DeckType dt; dt.name = "Test"; dt.maxCards = 2;
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    for (int i = 0; i < 3; ++i)
        deck.addCard(makeCard("c" + std::to_string(i), ct));

    std::string err;
    EXPECT_FALSE(deck.validate(err));
}

TEST(DeckTest, ValidateMaxCopies) {
    DeckType dt; dt.name = "Test"; dt.maxCopies = 2;
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    for (int i = 0; i < 3; ++i)
        deck.addCard(makeCard("same_id", ct)); // 3 copies of same id

    std::string err;
    EXPECT_FALSE(deck.validate(err));
}

TEST(DeckTest, ValidateAllowedCardTypes) {
    DeckType dt; dt.name = "Test"; dt.allowedCardTypes = {"Spell"};
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType(); // type is "Creature", not allowed
    deck.addCard(makeCard("c1", ct));

    std::string err;
    EXPECT_FALSE(deck.validate(err));
}

TEST(DeckTest, ValidPassesOnGoodDeck) {
    DeckType dt; dt.name = "Test"; dt.minCards = 1; dt.maxCards = 10; dt.maxCopies = 2;
    Deck deck("MyDeck", dt);
    auto ct = makeCreatureType();
    deck.addCard(makeCard("c1", ct));
    deck.addCard(makeCard("c2", ct));

    std::string err;
    EXPECT_TRUE(deck.validate(err));
}
