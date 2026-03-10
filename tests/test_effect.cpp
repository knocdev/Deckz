#include <gtest/gtest.h>
#include "engine/CardType.h"
#include "engine/EffectRegistry.h"
#include "engine/EffectContext.h"
#include "engine/GameState.h"

using namespace engine;

namespace {
    CardType spellType() {
        CardType ct("Spell");
        ct.addAttribute("cost",   AttributeType::Int, 0);
        ct.addAttribute("damage", AttributeType::Int, 0);
        return ct;
    }

    std::unique_ptr<Player> makePlayer(const std::string& name, const CardType& ct) {
        DeckType dt; dt.name = "Test";
        auto deck = std::make_unique<Deck>(name + "_deck", dt);
        for (int i = 0; i < 5; ++i)
            deck->addCard(std::make_shared<Card>(name + "_c" + std::to_string(i), ct));
        return std::make_unique<Player>(name, std::move(deck));
    }

    std::vector<Phase> phases() {
        return {
            { "Draw",   PhaseType::Draw,   1 },
            { "Action", PhaseType::Action, 0 },
            { "End",    PhaseType::End,    0 },
        };
    }
}

class EffectTest : public ::testing::Test {
protected:
    CardType ct = spellType();
    EffectRegistry reg;

    void SetUp() override {
        reg.registerBuiltins();

        std::vector<std::unique_ptr<Player>> players;
        players.push_back(makePlayer("Alice", ct));
        players.push_back(makePlayer("Bob",   ct));
        state = std::make_unique<GameState>(std::move(players), phases());
        alice = state->findPlayer("Alice");
        bob   = state->findPlayer("Bob");

        alice->setResource("health", 20);
        bob->setResource("health",   20);
        alice->setResource("mana",   10);
    }

    std::unique_ptr<GameState> state;
    Player* alice = nullptr;
    Player* bob   = nullptr;
};

TEST_F(EffectTest, DealDamageFixedAmount) {
    EffectDefinition def{ "deal_damage", EffectTrigger::OnPlay, { {"target","opponent"}, {"amount","5"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("c1", ct);
    EffectContext ctx{ *state, alice, card.get(), bob };
    fn(ctx);

    EXPECT_EQ(bob->getResource("health"), 15);
}

TEST_F(EffectTest, DealDamageFromCardAttribute) {
    EffectDefinition def{ "deal_damage", EffectTrigger::OnPlay, { {"target","opponent"}, {"amount","attr:damage"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("fireball", ct);
    card->setAttribute("damage", 7);
    EffectContext ctx{ *state, alice, card.get(), bob };
    fn(ctx);

    EXPECT_EQ(bob->getResource("health"), 13);
}

TEST_F(EffectTest, DealDamageSelf) {
    EffectDefinition def{ "deal_damage", EffectTrigger::OnPlay, { {"target","self"}, {"amount","3"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("c1", ct);
    EffectContext ctx{ *state, alice, card.get(), bob };
    fn(ctx);

    EXPECT_EQ(alice->getResource("health"), 17);
}

TEST_F(EffectTest, DrawCards) {
    EffectDefinition def{ "draw_cards", EffectTrigger::OnPlay, { {"count","2"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("c1", ct);
    EffectContext ctx{ *state, alice, card.get(), bob };

    EXPECT_EQ(alice->hand().size(), 0u);
    fn(ctx);
    EXPECT_EQ(alice->hand().size(), 2u);
    EXPECT_EQ(alice->deck().size(), 3u);
}

TEST_F(EffectTest, GainResource) {
    EffectDefinition def{ "gain_resource", EffectTrigger::OnPlay, { {"resource","mana"}, {"amount","3"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("c1", ct);
    EffectContext ctx{ *state, alice, card.get(), bob };
    fn(ctx);

    EXPECT_EQ(alice->getResource("mana"), 13);
}

TEST_F(EffectTest, LoseResource) {
    EffectDefinition def{ "lose_resource", EffectTrigger::OnPlay, { {"resource","mana"}, {"amount","4"} } };
    auto fn = reg.create(def);

    auto card = std::make_shared<Card>("c1", ct);
    EffectContext ctx{ *state, alice, card.get(), bob };
    fn(ctx);

    EXPECT_EQ(alice->getResource("mana"), 6);
}

TEST_F(EffectTest, UnknownEffectTypeThrows) {
    EffectDefinition def{ "unknown_effect", EffectTrigger::OnPlay, {} };
    EXPECT_THROW(reg.create(def), std::out_of_range);
}
