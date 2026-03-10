#pragma once

#include "Card.h"
#include <vector>
#include <memory>
#include <stdexcept>

namespace engine {

class Hand {
public:
    void addCard(std::shared_ptr<Card> card);

    // Remove and return card at index
    std::shared_ptr<Card> removeCard(size_t index);

    const std::vector<std::shared_ptr<Card>>& cards() const { return m_cards; }
    size_t size()  const { return m_cards.size(); }
    bool   empty() const { return m_cards.empty(); }

private:
    std::vector<std::shared_ptr<Card>> m_cards;
};

} // namespace engine
