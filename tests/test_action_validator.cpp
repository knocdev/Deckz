#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/GameState.h"
#include "engine/ActionValidator.h"

using namespace engine;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace {
    CardType creatureType() {
        CardType ct("Creature");
        ct.addAttribute("cost",   AttributeType::Int, 0);
        ct.addAttribute("attack", AttributeType::Int, 0);
        return ct;
    }

    std::unique_ptr<Player> makePlayer(const std::string& name,
                                       const CardType& ct, int cards) {
        DeckType dt; dt.name = "Test";
        auto deck = std::make_unique<Deck>(name + "_deck", dt);
        for (int i = 0; i < cards; ++i)
            deck->addCard(std::make_shared<Card>(name + "_c" + std::to_string(i), ct));
        return std::make_unique<Player>(name, std::move(deck));
    }

    // Phases: Draw → Action → End
    std::vector<Phase> phases() {
        return {
            { "Draw",   PhaseType::Draw,   1 },
            { "Action", PhaseType::Action, 0 },
            { "End",    PhaseType::End,    0 },
        };
    }
}

// ---------------------------------------------------------------------------
// Fixture — two players, Alice starts on Draw phase
// ---------------------------------------------------------------------------

class ActionValidatorTest : public ::testing::Test {
protected:
    CardType ct = creatureType();

    void SetUp() override {
        std::vector<std::unique_ptr<Player>> players;
        players.push_back(makePlayer("Alice", ct, 3));
        players.push_back(makePlayer("Bob",   ct, 3));
        state = std::make_unique<GameState>(std::move(players), phases());
        alice = state->findPlayer("Alice");
        bob   = state->findPlayer("Bob");
    }

    std::unique_ptr<GameState> state;
    Player* alice = nullptr;
    Player* bob   = nullptr;
};

// ---------------------------------------------------------------------------
// actorNotNull
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, ActorNotNullFailsOnNull) {
    ActionValidator v;
    v.addGlobalRule(Rules::actorNotNull());

    Action a{ ActionType::EndPhase, nullptr };
    auto result = v.validate(a, *state);
    EXPECT_FALSE(result);
    EXPECT_FALSE(result.reason.empty());
}

TEST_F(ActionValidatorTest, ActorNotNullPassesWithActor) {
    ActionValidator v;
    v.addGlobalRule(Rules::actorNotNull());

    Action a{ ActionType::EndPhase, alice };
    EXPECT_TRUE(v.validate(a, *state));
}

// ---------------------------------------------------------------------------
// isCurrentPlayer
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, IsCurrentPlayerPassesForAlice) {
    ActionValidator v;
    v.addGlobalRule(Rules::isCurrentPlayer());

    Action a{ ActionType::EndPhase, alice };
    EXPECT_TRUE(v.validate(a, *state));
}

TEST_F(ActionValidatorTest, IsCurrentPlayerFailsForBob) {
    ActionValidator v;
    v.addGlobalRule(Rules::isCurrentPlayer());

    Action a{ ActionType::EndPhase, bob };
    auto result = v.validate(a, *state);
    EXPECT_FALSE(result);
}

// ---------------------------------------------------------------------------
// phaseIs
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, PhaseIsDrawPassesOnDrawPhase) {
    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::phaseIs(PhaseType::Draw));

    Action a{ ActionType::PlayCard, alice };
    EXPECT_TRUE(v.validate(a, *state)); // currently Draw phase
}

TEST_F(ActionValidatorTest, PhaseIsActionFailsOnDrawPhase) {
    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::phaseIs(PhaseType::Action));

    Action a{ ActionType::PlayCard, alice };
    auto result = v.validate(a, *state); // currently Draw phase
    EXPECT_FALSE(result);
}

// ---------------------------------------------------------------------------
// phaseIsOneOf
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, PhaseIsOneOfPassesWhenMatches) {
    ActionValidator v;
    v.addRule(ActionType::PlayCard,
              Rules::phaseIsOneOf({ PhaseType::Draw, PhaseType::Action }));

    Action a{ ActionType::PlayCard, alice };
    EXPECT_TRUE(v.validate(a, *state)); // Draw phase — matches
}

TEST_F(ActionValidatorTest, PhaseIsOneOfFailsWhenNoMatch) {
    ActionValidator v;
    v.addRule(ActionType::PlayCard,
              Rules::phaseIsOneOf({ PhaseType::Combat, PhaseType::End }));

    Action a{ ActionType::PlayCard, alice };
    EXPECT_FALSE(v.validate(a, *state));
}

// ---------------------------------------------------------------------------
// handIndexInRange
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, HandIndexInRangeFailsOnEmptyHand) {
    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::handIndexInRange());

    Action a{ ActionType::PlayCard, alice, 0 }; // hand is empty at start
    EXPECT_FALSE(v.validate(a, *state));
}

TEST_F(ActionValidatorTest, HandIndexInRangePassesAfterDraw) {
    alice->hand().addCard(std::make_shared<Card>("test_card", ct));

    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::handIndexInRange());

    Action a{ ActionType::PlayCard, alice, 0 };
    EXPECT_TRUE(v.validate(a, *state));
}

// ---------------------------------------------------------------------------
// hasEnoughResource
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, HasEnoughResourcePassesWithSufficientMana) {
    auto card = std::make_shared<Card>("spell", ct);
    card->setAttribute("cost", 3);
    alice->hand().addCard(card);
    alice->setResource("mana", 5);

    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::hasEnoughResource("mana", "cost"));

    Action a{ ActionType::PlayCard, alice, 0 };
    EXPECT_TRUE(v.validate(a, *state));
}

TEST_F(ActionValidatorTest, HasEnoughResourceFailsWithInsufficientMana) {
    auto card = std::make_shared<Card>("spell", ct);
    card->setAttribute("cost", 5);
    alice->hand().addCard(card);
    alice->setResource("mana", 2);

    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::hasEnoughResource("mana", "cost"));

    Action a{ ActionType::PlayCard, alice, 0 };
    auto result = v.validate(a, *state);
    EXPECT_FALSE(result);
    EXPECT_NE(result.reason.find("mana"), std::string::npos);
}

TEST_F(ActionValidatorTest, HasEnoughResourceFailsWithNoResourceSet) {
    auto card = std::make_shared<Card>("spell", ct);
    card->setAttribute("cost", 1);
    alice->hand().addCard(card);
    // mana not set on alice

    ActionValidator v;
    v.addRule(ActionType::PlayCard, Rules::hasEnoughResource("mana", "cost"));

    Action a{ ActionType::PlayCard, alice, 0 };
    EXPECT_FALSE(v.validate(a, *state));
}

// ---------------------------------------------------------------------------
// Rules only run for their registered action type
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, RuleForPlayCardDoesNotRunForEndPhase) {
    ActionValidator v;
    // This rule would fail (empty hand), but it's only for PlayCard
    v.addRule(ActionType::PlayCard, Rules::handIndexInRange());

    Action a{ ActionType::EndPhase, alice };
    EXPECT_TRUE(v.validate(a, *state)); // EndPhase has no rules — passes
}

// ---------------------------------------------------------------------------
// Global rules run for all action types
// ---------------------------------------------------------------------------

TEST_F(ActionValidatorTest, GlobalRuleRunsForAllActionTypes) {
    ActionValidator v;
    v.addGlobalRule(Rules::isCurrentPlayer());

    EXPECT_FALSE(v.validate({ ActionType::PlayCard,  bob }, *state));
    EXPECT_FALSE(v.validate({ ActionType::EndPhase,  bob }, *state));
    EXPECT_FALSE(v.validate({ ActionType::Attack,    bob }, *state));
}
