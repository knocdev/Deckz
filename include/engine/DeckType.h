#pragma once

#include <string>
#include <vector>

namespace engine {

// Rules and constraints for a class of decks
struct DeckType {
    std::string name;

    // Card type names allowed in this deck; empty = all allowed
    std::vector<std::string> allowedCardTypes;

    int minCards  = 0;   // minimum deck size (0 = no minimum)
    int maxCards  = -1;  // maximum deck size (-1 = unlimited)
    int maxCopies = -1;  // max copies of any single card id (-1 = unlimited)
};

} // namespace engine
