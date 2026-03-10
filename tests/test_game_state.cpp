#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/GameState.h"

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

    std::unique_ptr<Player> makePlayer(const std::string& name,
                                       const CardType& ct, int cardCount) {
        DeckType dt; dt.name = "Test";
        auto deck = std::make_unique<Deck>(name + "_deck", dt);
        for (int i = 0; i < cardCount; ++i)
            deck->addCard(std::make_shared<Card>(name + "_c" + std::to_string(i), ct));
        return std::make_unique<Player>(name, std::move(deck));
    }

    std::vector<Phase> threePhases() {
        return {
            { "Draw", PhaseType::Draw,   1 },
            { "Main", PhaseType::Action, 0 },
            { "End",  PhaseType::End,    0 },
        };
    }
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST(GameStateTest, StartsInProgress) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 5));

    GameState gs(std::move(players), threePhases());
    EXPECT_EQ(gs.status(), GameStatus::InProgress);
    EXPECT_FALSE(gs.isOver());
    EXPECT_EQ(gs.winner(), nullptr);
}

TEST(GameStateTest, FindPlayerByName) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 3));
    players.push_back(makePlayer("Bob",   ct, 3));

    GameState gs(std::move(players), threePhases());
    EXPECT_NE(gs.findPlayer("Alice"), nullptr);
    EXPECT_NE(gs.findPlayer("Bob"),   nullptr);
    EXPECT_EQ(gs.findPlayer("Eve"),   nullptr);
}

TEST(GameStateTest, StepExecutesDrawAndAdvances) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 5));

    GameState gs(std::move(players), threePhases());
    Player* alice = gs.findPlayer("Alice");

    EXPECT_EQ(alice->hand().size(), 0u);
    gs.step(); // Draw phase: draw 1, then advance to Main
    EXPECT_EQ(alice->hand().size(), 1u);
    EXPECT_EQ(gs.turnEngine().currentPhase().name, "Main");
}

TEST(GameStateTest, WinConditionTriggersGameOver) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 5));
    players.push_back(makePlayer("Bob",   ct, 5));

    GameState gs(std::move(players), threePhases());
    Player* alice = gs.findPlayer("Alice");

    // Alice wins immediately on first check
    gs.addWinCondition([alice](const GameState&) -> WinCheckResult {
        return { true, alice };
    });

    gs.step();

    EXPECT_TRUE(gs.isOver());
    EXPECT_EQ(gs.status(), GameStatus::Won);
    EXPECT_EQ(gs.winner(), alice);
}

TEST(GameStateTest, DrawConditionSetsDrawStatus) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 5));

    GameState gs(std::move(players), threePhases());

    gs.addWinCondition([](const GameState&) -> WinCheckResult {
        return { true, nullptr }; // game over, no winner = draw
    });

    gs.step();

    EXPECT_EQ(gs.status(), GameStatus::Draw);
    EXPECT_EQ(gs.winner(), nullptr);
}

TEST(GameStateTest, StepAfterGameOverDoesNothing) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 5));

    GameState gs(std::move(players), threePhases());
    Player* alice = gs.findPlayer("Alice");
    gs.addWinCondition([alice](const GameState&) -> WinCheckResult {
        return { true, alice };
    });

    gs.step(); // ends game
    int turnBefore = gs.turnEngine().turnNumber();
    gs.step(); // should be a no-op
    gs.step();

    EXPECT_EQ(gs.turnEngine().turnNumber(), turnBefore);
}

TEST(GameStateTest, RunUntilOverStopsOnWin) {
    auto ct = creatureType();
    std::vector<std::unique_ptr<Player>> players;
    players.push_back(makePlayer("Alice", ct, 3));
    players.push_back(makePlayer("Bob",   ct, 3));

    GameState gs(std::move(players), threePhases());

    // Bob wins when it's turn 2
    Player* bob = gs.findPlayer("Bob");
    gs.addWinCondition([bob](const GameState& state) -> WinCheckResult {
        if (state.turnEngine().turnNumber() >= 2)
            return { true, bob };
        return {};
    });

    gs.runUntilOver();

    EXPECT_TRUE(gs.isOver());
    EXPECT_EQ(gs.winner(), bob);
}
