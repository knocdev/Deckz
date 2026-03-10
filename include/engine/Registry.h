#pragma once

#include "CardType.h"
#include "DeckType.h"
#include "Deck.h"
#include "Effect.h"
#include "Phase.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <stdexcept>

namespace engine {

struct CardDefinition {
    std::string id;
    std::string typeName;
    std::unordered_map<std::string, AttributeValue> attributes;
};

// Owns all type definitions loaded from a JSON config.
// Acts as a factory for Card and Deck instances.
class Registry {
public:
    void loadFromFile(const std::string& path);

    // --- Type accessors ---
    const CardType& cardType(const std::string& name) const;
    const DeckType& deckType(const std::string& name) const;
    const std::vector<Phase>&           phases()           const { return m_phases; }
    const std::vector<EffectDefinition>& turnStartEffects() const { return m_turnStartEffects; }
    const std::vector<EffectDefinition>& turnEndEffects()   const { return m_turnEndEffects; }

    bool hasCardType(const std::string& name) const;
    bool hasDeckType(const std::string& name) const;

    // --- Card definition lookup ---
    const std::string& cardIdToTypeName(const std::string& id) const;

    // --- Factory methods ---
    // Creates a card with default attribute values from its CardType schema,
    // then overlays any definition attributes if the card id is known.
    std::shared_ptr<Card> createCard(const std::string& id,
                                     const std::string& cardTypeName) const;

    std::unique_ptr<Deck> createDeck(const std::string& name,
                                     const std::string& deckTypeName) const;

private:
    std::unordered_map<std::string, CardType>       m_cardTypes;
    std::unordered_map<std::string, DeckType>       m_deckTypes;
    std::unordered_map<std::string, CardDefinition> m_cardDefinitions;
    std::vector<Phase>           m_phases;
    std::vector<EffectDefinition> m_turnStartEffects;
    std::vector<EffectDefinition> m_turnEndEffects;
};

} // namespace engine
