#pragma once

#include "Card.h"
#include "DeckType.h"
#include <vector>
#include <memory>
#include <string>

namespace engine {

class Deck {
public:
    Deck(std::string name, const DeckType& type);

    const std::string& name() const { return m_name; }
    const DeckType& type() const { return m_type; }

    void addCard(std::shared_ptr<Card> card);

    // Draw from top (index 0) or bottom
    std::shared_ptr<Card> drawTop();
    std::shared_ptr<Card> drawBottom();

    void shuffle();

    // Returns false + a reason string if the deck violates its DeckType rules
    bool validate(std::string& outError) const;

    size_t size() const { return m_cards.size(); }
    bool empty() const { return m_cards.empty(); }

    const std::vector<std::shared_ptr<Card>>& cards() const { return m_cards; }

private:
    std::string m_name;
    const DeckType& m_type;
    std::vector<std::shared_ptr<Card>> m_cards;
};

} // namespace engine
