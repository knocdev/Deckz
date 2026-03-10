#include "engine/Deck.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

namespace {
    std::string itos(int n) { std::ostringstream s; s << n; return s.str(); }
}

namespace engine {

Deck::Deck(std::string name, const DeckType& type)
    : m_name(std::move(name)), m_type(type) {}

void Deck::addCard(std::shared_ptr<Card> card) {
    m_cards.push_back(std::move(card));
}

std::shared_ptr<Card> Deck::drawTop() {
    if (m_cards.empty())
        throw std::underflow_error("Cannot draw from an empty deck");
    auto card = m_cards.front();
    m_cards.erase(m_cards.begin());
    return card;
}

std::shared_ptr<Card> Deck::drawBottom() {
    if (m_cards.empty())
        throw std::underflow_error("Cannot draw from an empty deck");
    auto card = m_cards.back();
    m_cards.pop_back();
    return card;
}

void Deck::shuffle() {
    static std::mt19937 rng{ std::random_device{}() };
    std::shuffle(m_cards.begin(), m_cards.end(), rng);
}

bool Deck::validate(std::string& outError) const {
    const int count = static_cast<int>(m_cards.size());

    if (m_type.minCards > 0 && count < m_type.minCards) {
        outError = "Deck '" + m_name + "' has " + itos(count) +
                   " cards, minimum is " + itos(m_type.minCards);
        return false;
    }

    if (m_type.maxCards >= 0 && count > m_type.maxCards) {
        outError = "Deck '" + m_name + "' has " + itos(count) +
                   " cards, maximum is " + itos(m_type.maxCards);
        return false;
    }

    // Check allowed card types
    if (!m_type.allowedCardTypes.empty()) {
        for (const auto& card : m_cards) {
            const std::string& typeName = card->type().name();
            bool allowed = false;
            for (const auto& allowed_type : m_type.allowedCardTypes)
                if (allowed_type == typeName) { allowed = true; break; }
            if (!allowed) {
                outError = "Card '" + card->id() + "' has type '" + typeName +
                           "' which is not allowed in deck type '" + m_type.name + "'";
                return false;
            }
        }
    }

    // Check max copies
    if (m_type.maxCopies >= 0) {
        std::unordered_map<std::string, int> copies;
        for (const auto& card : m_cards)
            copies[card->id()]++;
        for (const auto& [id, cnt] : copies) {
            if (cnt > m_type.maxCopies) {
                outError = "Card '" + id + "' appears " + itos(cnt) +
                           " times, max copies is " + itos(m_type.maxCopies);
                return false;
            }
        }
    }

    return true;
}

} // namespace engine
