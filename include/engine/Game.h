#pragma once

#include "Registry.h"
#include "EffectRegistry.h"
#include "GameState.h"
#include "ActionValidator.h"
#include <vector>
#include <string>
#include <memory>

namespace engine {

// High-level facade: load config → add players → start → submit actions
class Game {
public:
    explicit Game(const std::string& configPath);

    // --- Setup (call before start()) ---
    void registerEffect(const std::string& typeName, EffectFactory factory);
    void addWinCondition(GameState::WinCondition condition);
    ActionValidator& validator() { return m_validator; }

    // Add a player whose deck contains copies of the given card IDs (from config)
    void addPlayer(const std::string& playerName,
                   const std::string& deckTypeName,
                   const std::vector<std::string>& cardIds);

    // --- Start ---
    void start();

    // --- Per-turn ---
    // Validate and, if valid, execute the action (triggers effects, deducts costs)
    ValidationResult submitAction(const Action& action);

    // --- Query ---
    GameState&       state();
    const GameState& state()    const;
    const Registry&  registry() const { return m_registry; }
    bool             isOver()   const;

private:
    Registry        m_registry;
    EffectRegistry  m_effectRegistry;
    ActionValidator m_validator;

    struct PlayerSetup {
        std::string              name;
        std::string              deckTypeName;
        std::vector<std::string> cardIds;
    };

    std::vector<PlayerSetup>             m_playerSetups;
    std::vector<GameState::WinCondition> m_winConditions;
    std::unique_ptr<GameState>           m_state;

    void executePlayCard(Action& action);
    void triggerEffects(EffectTrigger trigger, Player* actor, Card* card = nullptr);
    Player* findOpponent(Player* actor) const;

    ActionValidator::Rule      buildRule(const RuleDefinition& def) const;
    GameState::WinCondition    buildWinCondition(const WinConditionDefinition& def) const;
};

} // namespace engine
