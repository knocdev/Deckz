#include "engine/Hand.h"

namespace engine {

void Hand::addCard(std::shared_ptr<Card> card) {
    m_cards.push_back(std::move(card));
}

std::shared_ptr<Card> Hand::removeCard(size_t index) {
    if (index >= m_cards.size())
        throw std::out_of_range("Hand index out of range");
    auto card = m_cards[index];
    m_cards.erase(m_cards.begin() + static_cast<ptrdiff_t>(index));
    return card;
}

} // namespace engine
