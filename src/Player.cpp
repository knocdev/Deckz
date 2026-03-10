#include "engine/Player.h"
#include <stdexcept>

namespace engine {

Player::Player(std::string name, std::unique_ptr<Deck> deck)
    : m_name(std::move(name)), m_deck(std::move(deck)) {}

void Player::setResource(const std::string& key, int value) {
    m_resources[key] = value;
}

int Player::getResource(const std::string& key) const {
    auto it = m_resources.find(key);
    if (it == m_resources.end())
        throw std::out_of_range("Player '" + m_name + "' has no resource '" + key + "'");
    return it->second;
}

bool Player::hasResource(const std::string& key) const {
    return m_resources.count(key) > 0;
}

} // namespace engine
