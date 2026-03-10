#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/Player.h"
#include "engine/TurnEngine.h"

using namespace engine;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {
    CardType creatureType() {
        CardType ct("Creature");
        ct.addAttribute("attack", AttributeType::Int, 0);
        return ct;
    }

    std::unique_ptr<Deck> deckWithCards(const CardType& ct, int count) {
        DeckType dt; dt.name = "Test";
        auto deck = std::make_unique<Deck>("deck", dt);
        for (int i = 0; i < count; ++i)
            deck->addCard(std::make_shared<Card>("c" + std::to_string(i), ct));
        return deck;
    }

    std::vector<Phase> standardPhases() {
        return {
            { "Draw",   PhaseType::Draw,   1 },
            { "Main",   PhaseType::Action, 0 },
            { "End",    PhaseType::End,    0 },
        };
    }
}

// ---------------------------------------------------------------------------
// TurnEngine tests
// ---------------------------------------------------------------------------

TEST(TurnEngineTest, StartsAtFirstPhaseAndPlayer) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    EXPECT_EQ(engine.currentPhase().name, "Draw");
    EXPECT_EQ(engine.currentPlayer().name(), "Alice");
    EXPECT_EQ(engine.turnNumber(), 1);
}

TEST(TurnEngineTest, DrawPhaseMovesCardToHand) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    EXPECT_EQ(p1.hand().size(), 0u);
    engine.executeCurrentPhase(); // Draw phase
    EXPECT_EQ(p1.hand().size(), 1u);
    EXPECT_EQ(p1.deck().size(), 4u);
}

TEST(TurnEngineTest, AdvancePhaseMovesToNext) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    engine.advancePhase();
    EXPECT_EQ(engine.currentPhase().name, "Main");
    engine.advancePhase();
    EXPECT_EQ(engine.currentPhase().name, "End");
}

TEST(TurnEngineTest, AdvancePastLastPhaseWrapsToFirstAndNewTurn) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    // Advance through all 3 phases
    engine.advancePhase();
    engine.advancePhase();
    engine.advancePhase(); // wraps back to Draw, turn 2

    EXPECT_EQ(engine.currentPhase().name, "Draw");
    EXPECT_EQ(engine.turnNumber(), 2);
}

TEST(TurnEngineTest, MultiPlayerRotation) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    Player p2("Bob",   deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1, &p2 });

    EXPECT_EQ(engine.currentPlayer().name(), "Alice");

    // Advance through Alice's 3 phases
    engine.advancePhase();
    engine.advancePhase();
    engine.advancePhase(); // now Bob's turn

    EXPECT_EQ(engine.currentPlayer().name(), "Bob");
    EXPECT_EQ(engine.turnNumber(), 1); // turn increments after all players done

    // Advance through Bob's 3 phases
    engine.advancePhase();
    engine.advancePhase();
    engine.advancePhase(); // back to Alice, turn 2

    EXPECT_EQ(engine.currentPlayer().name(), "Alice");
    EXPECT_EQ(engine.turnNumber(), 2);
}

TEST(TurnEngineTest, DrawOnEmptyDeckDoesNotCrash) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 0)); // empty deck
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    EXPECT_NO_THROW(engine.executeCurrentPhase());
    EXPECT_EQ(p1.hand().size(), 0u);
}

TEST(TurnEngineTest, PhaseStartCallbackFires) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 3));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    std::vector<std::string> log;
    engine.onPhaseStart([&](const PhaseEvent& e) {
        log.push_back(e.phase.name);
    });

    engine.advancePhase(); // fires Main phase start
    engine.advancePhase(); // fires End phase start

    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], "Main");
    EXPECT_EQ(log[1], "End");
}

TEST(TurnEngineTest, TurnStartCallbackFires) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();

    std::vector<int> turns;
    TurnEngine engine(phases, { &p1 });
    engine.onTurnStart([&](Player&, int t) { turns.push_back(t); });

    // Advance through all phases to trigger turn 2
    engine.advancePhase();
    engine.advancePhase();
    engine.advancePhase();

    ASSERT_EQ(turns.size(), 1u);
    EXPECT_EQ(turns[0], 2);
}

TEST(TurnEngineTest, EndGameStopsExecution) {
    auto ct = creatureType();
    Player p1("Alice", deckWithCards(ct, 5));
    auto phases = standardPhases();
    TurnEngine engine(phases, { &p1 });

    engine.endGame();
    engine.executeCurrentPhase();
    engine.advancePhase();

    // Phase and turn should not have advanced
    EXPECT_EQ(engine.currentPhase().name, "Draw");
    EXPECT_TRUE(engine.isGameOver());
}
