#pragma once

#include "Deck.h"
#include "Hand.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace engine {

class Player {
public:
    Player(std::string name, std::unique_ptr<Deck> deck);

    const std::string& name() const { return m_name; }
    Deck& deck()             { return *m_deck; }
    const Deck& deck() const { return *m_deck; }
    Hand& hand()             { return m_hand; }
    const Hand& hand() const { return m_hand; }

    // Generic integer resources (health, mana, gold, etc.)
    void setResource(const std::string& key, int value);
    int  getResource(const std::string& key) const; // throws if missing
    bool hasResource(const std::string& key) const;

private:
    std::string m_name;
    std::unique_ptr<Deck> m_deck;
    Hand m_hand;
    std::unordered_map<std::string, int> m_resources;
};

} // namespace engine
