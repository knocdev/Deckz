#include <gtest/gtest.h>
#include "engine/Game.h"

using namespace engine;

static const std::string CONFIG = "examples/basic/config.json";

class GameTest : public ::testing::Test {
protected:
    void SetUp() override {
        game = std::make_unique<Game>(CONFIG);

        // Standard rules: must be your turn, in Action phase, hand not empty, enough mana
        game->validator().addGlobalRule(Rules::actorNotNull());
        game->validator().addGlobalRule(Rules::isCurrentPlayer());
        game->validator().addRule(ActionType::PlayCard, Rules::phaseIs(PhaseType::Action));
        game->validator().addRule(ActionType::PlayCard, Rules::handIndexInRange());
        game->validator().addRule(ActionType::PlayCard, Rules::hasEnoughResource("mana", "cost"));

        // Win condition: player with health <= 0 loses
        game->addWinCondition([](const GameState& state) -> WinCheckResult {
            for (const auto& p : state.players()) {
                if (p->hasResource("health") && p->getResource("health") <= 0) {
                    // find the opponent
                    for (const auto& other : state.players()) {
                        if (other.get() != p.get())
                            return { true, other.get() };
                    }
                    return { true, nullptr };
                }
            }
            return {};
        });

        game->addPlayer("Alice", "StandardDeck", {"fireball", "fireball", "fireball",
                                                   "fireball", "fireball", "fireball",
                                                   "fireball", "fireball", "fireball",
                                                   "fireball"});
        game->addPlayer("Bob",   "StandardDeck", {"dragon",   "dragon",   "dragon",
                                                   "dragon",   "dragon",   "dragon",
                                                   "dragon",   "dragon",   "dragon",
                                                   "dragon"});
        game->start();

        alice = game->state().findPlayer("Alice");
        bob   = game->state().findPlayer("Bob");
        alice->setResource("health", 20);
        bob->setResource("health",   20);
        alice->setResource("mana",   10);
        bob->setResource("mana",     10);
    }

    std::unique_ptr<Game> game;
    Player* alice = nullptr;
    Player* bob   = nullptr;
};

TEST_F(GameTest, GameStartsInProgress) {
    EXPECT_FALSE(game->isOver());
    EXPECT_EQ(game->state().status(), GameStatus::InProgress);
}

TEST_F(GameTest, CannotPlayCardOnWrongPhase) {
    // Currently Draw phase
    Action a{ ActionType::PlayCard, alice, 0 };
    auto result = game->submitAction(a);
    EXPECT_FALSE(result);
}

TEST_F(GameTest, CannotPlayCardWhenNotYourTurn) {
    // Advance to Action phase
    game->state().turnEngine().advancePhase();

    // Bob tries to play on Alice's turn
    bob->hand().addCard(game->registry().createCard("fireball", "Spell"));
    Action a{ ActionType::PlayCard, bob, 0 };
    EXPECT_FALSE(game->submitAction(a));
}

TEST_F(GameTest, PlayCardDeductsManaCost) {
    // Draw phase → Action phase
    game->state().turnEngine().advancePhase();

    auto card = game->registry().createCard("fireball_test", "Spell");
    card->setAttribute("cost", 3);
    alice->hand().addCard(card);

    Action a{ ActionType::PlayCard, alice, 0 };
    auto result = game->submitAction(a);
    EXPECT_TRUE(result);
    EXPECT_EQ(alice->getResource("mana"), 7);
}

TEST_F(GameTest, CannotPlayWithInsufficientMana) {
    game->state().turnEngine().advancePhase();

    auto card = game->registry().createCard("expensive", "Spell");
    card->setAttribute("cost", 15);
    alice->hand().addCard(card);

    Action a{ ActionType::PlayCard, alice, 0 };
    EXPECT_FALSE(game->submitAction(a));
}

TEST_F(GameTest, PlayCardRemovesFromHand) {
    game->state().turnEngine().advancePhase();

    auto card = game->registry().createCard("cheap_spell", "Spell");
    card->setAttribute("cost", 1);
    alice->hand().addCard(card);
    EXPECT_EQ(alice->hand().size(), 1u);

    Action a{ ActionType::PlayCard, alice, 0 };
    game->submitAction(a);
    EXPECT_EQ(alice->hand().size(), 0u);
}
