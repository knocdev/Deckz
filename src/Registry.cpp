#include "engine/Registry.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <stdexcept>

using json = nlohmann::json;

namespace engine {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static AttributeValue defaultValueFor(AttributeType type) {
    switch (type) {
        case AttributeType::Int:    return 0;
        case AttributeType::Float:  return 0.0f;
        case AttributeType::Bool:   return false;
        case AttributeType::String: return std::string{};
    }
    return 0;
}

static AttributeValue jsonToAttributeValue(const json& j, AttributeType type) {
    switch (type) {
        case AttributeType::Int:    return j.get<int>();
        case AttributeType::Float:  return j.get<float>();
        case AttributeType::Bool:   return j.get<bool>();
        case AttributeType::String: return j.get<std::string>();
    }
    return 0;
}

// Convert a JSON value (int or string) to string for EffectParams storage
static std::string jsonToParamString(const json& j) {
    if (j.is_string()) return j.get<std::string>();
    if (j.is_number_integer()) return std::to_string(j.get<int>());
    if (j.is_number_float()) return std::to_string(j.get<float>());
    if (j.is_boolean()) return j.get<bool>() ? "true" : "false";
    return j.dump();
}

// ---------------------------------------------------------------------------
// Parsing
// ---------------------------------------------------------------------------

static EffectDefinition parseEffectDef(const json& eff) {
    EffectDefinition def;
    def.typeName = eff.at("type").get<std::string>();
    if (eff.contains("trigger"))
        def.trigger = effectTriggerFromString(eff.at("trigger").get<std::string>());
    if (eff.contains("params"))
        for (const auto& [k, v] : eff["params"].items())
            def.params[k] = jsonToParamString(v);
    return def;
}

static void parseCardTypes(const json& j,
                            std::unordered_map<std::string, CardType>& out) {
    for (const auto& ct : j) {
        const std::string name = ct.at("name").get<std::string>();
        CardType cardType(name);

        if (ct.contains("attributes")) {
            for (const auto& [attrName, attrDef] : ct["attributes"].items()) {
                const std::string typeStr = attrDef.at("type").get<std::string>();
                AttributeType attrType = attributeTypeFromString(typeStr);

                AttributeValue defVal = attrDef.contains("default")
                    ? jsonToAttributeValue(attrDef["default"], attrType)
                    : defaultValueFor(attrType);

                cardType.addAttribute(attrName, attrType, defVal);
            }
        }

        if (ct.contains("effects"))
            for (const auto& eff : ct["effects"])
                cardType.addEffect(parseEffectDef(eff));

        out.emplace(name, std::move(cardType));
    }
}

static void parseTurnEffects(const json& j, std::vector<EffectDefinition>& out) {
    for (const auto& eff : j)
        out.push_back(parseEffectDef(eff));
}

static void parsePlayers(const json& j, std::vector<PlayerDefinition>& out) {
    for (const auto& p : j) {
        PlayerDefinition def;
        def.name         = p.at("name").get<std::string>();
        def.deckTypeName = p.at("deck_type").get<std::string>();
        def.cardIds      = p.at("cards").get<std::vector<std::string>>();
        if (p.contains("starting_resources"))
            for (const auto& [k, v] : p["starting_resources"].items())
                def.startingResources[k] = v.get<int>();
        out.push_back(std::move(def));
    }
}

static RuleDefinition parseRuleDef(const json& r) {
    RuleDefinition def;
    def.typeName = r.at("type").get<std::string>();
    if (r.contains("allowed")) {
        // join array of strings as comma-separated
        std::string joined;
        for (const auto& s : r["allowed"]) {
            if (!joined.empty()) joined += ',';
            joined += s.get<std::string>();
        }
        def.params["allowed"] = joined;
    }
    for (const auto& [k, v] : r.items()) {
        if (k == "type" || k == "allowed") continue;
        def.params[k] = jsonToParamString(v);
    }
    return def;
}

static void parseRules(const json& j,
                        std::vector<RuleDefinition>& globalOut,
                        std::unordered_map<std::string, std::vector<RuleDefinition>>& actionOut) {
    if (j.contains("global"))
        for (const auto& r : j["global"])
            globalOut.push_back(parseRuleDef(r));
    for (const auto& [key, arr] : j.items()) {
        if (key == "global") continue;
        for (const auto& r : arr)
            actionOut[key].push_back(parseRuleDef(r));
    }
}

static void parseWinConditions(const json& j, std::vector<WinConditionDefinition>& out) {
    for (const auto& w : j) {
        WinConditionDefinition def;
        def.typeName = w.at("type").get<std::string>();
        for (const auto& [k, v] : w.items()) {
            if (k == "type") continue;
            def.params[k] = jsonToParamString(v);
        }
        out.push_back(std::move(def));
    }
}

static void parseDeckTypes(const json& j,
                            std::unordered_map<std::string, DeckType>& out) {
    for (const auto& dt : j) {
        DeckType deckType;
        deckType.name = dt.at("name").get<std::string>();

        if (dt.contains("allowed_card_types"))
            deckType.allowedCardTypes = dt["allowed_card_types"].get<std::vector<std::string>>();

        if (dt.contains("min_cards")) deckType.minCards  = dt["min_cards"].get<int>();
        if (dt.contains("max_cards")) deckType.maxCards  = dt["max_cards"].get<int>();
        if (dt.contains("max_copies")) deckType.maxCopies = dt["max_copies"].get<int>();

        out.emplace(deckType.name, std::move(deckType));
    }
}

static void parsePhases(const json& j, std::vector<Phase>& out) {
    for (const auto& p : j) {
        Phase phase;
        phase.name = p.at("name").get<std::string>();
        phase.type = phaseTypeFromString(p.at("type").get<std::string>());
        if (p.contains("draw_count"))
            phase.drawCount = p["draw_count"].get<int>();
        out.push_back(std::move(phase));
    }
}

static void parseCards(const json& j,
                       const std::unordered_map<std::string, CardType>& cardTypes,
                       std::unordered_map<std::string, CardDefinition>& out) {
    for (const auto& c : j) {
        CardDefinition def;
        def.id       = c.at("id").get<std::string>();
        def.typeName = c.at("type").get<std::string>();

        if (c.contains("attributes")) {
            // Look up the CardType to know the attribute types for proper conversion
            auto typeIt = cardTypes.find(def.typeName);
            for (const auto& [key, val] : c["attributes"].items()) {
                if (typeIt != cardTypes.end() && typeIt->second.hasAttribute(key)) {
                    AttributeType attrType = typeIt->second.getAttribute(key).type;
                    def.attributes[key] = jsonToAttributeValue(val, attrType);
                } else {
                    // Best-effort: treat numbers as int, otherwise string
                    if (val.is_number_integer())      def.attributes[key] = val.get<int>();
                    else if (val.is_number_float())   def.attributes[key] = val.get<float>();
                    else if (val.is_boolean())        def.attributes[key] = val.get<bool>();
                    else if (val.is_string())         def.attributes[key] = val.get<std::string>();
                }
            }
        }

        out.emplace(def.id, std::move(def));
    }
}

// ---------------------------------------------------------------------------
// Public interface
// ---------------------------------------------------------------------------

void Registry::loadFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Could not open config file: " + path);

    json j;
    file >> j;

    if (j.contains("card_types"))         parseCardTypes(j["card_types"], m_cardTypes);
    if (j.contains("deck_types"))         parseDeckTypes(j["deck_types"], m_deckTypes);
    if (j.contains("phases"))             parsePhases(j["phases"], m_phases);
    if (j.contains("cards"))             parseCards(j["cards"], m_cardTypes, m_cardDefinitions);
    if (j.contains("turn_start_effects")) parseTurnEffects(j["turn_start_effects"], m_turnStartEffects);
    if (j.contains("turn_end_effects"))   parseTurnEffects(j["turn_end_effects"],   m_turnEndEffects);
    if (j.contains("players"))           parsePlayers(j["players"], m_players);
    if (j.contains("rules"))             parseRules(j["rules"], m_globalRules, m_actionRules);
    if (j.contains("win_conditions"))    parseWinConditions(j["win_conditions"], m_winConditions);
}

const std::vector<RuleDefinition>& Registry::actionRules(const std::string& actionType) const {
    static const std::vector<RuleDefinition> empty;
    auto it = m_actionRules.find(actionType);
    return it != m_actionRules.end() ? it->second : empty;
}

const CardType& Registry::cardType(const std::string& name) const {
    auto it = m_cardTypes.find(name);
    if (it == m_cardTypes.end())
        throw std::out_of_range("Unknown card type: " + name);
    return it->second;
}

const DeckType& Registry::deckType(const std::string& name) const {
    auto it = m_deckTypes.find(name);
    if (it == m_deckTypes.end())
        throw std::out_of_range("Unknown deck type: " + name);
    return it->second;
}

bool Registry::hasCardType(const std::string& name) const {
    return m_cardTypes.count(name) > 0;
}

bool Registry::hasDeckType(const std::string& name) const {
    return m_deckTypes.count(name) > 0;
}

const std::string& Registry::cardIdToTypeName(const std::string& id) const {
    auto it = m_cardDefinitions.find(id);
    if (it == m_cardDefinitions.end())
        throw std::out_of_range("Unknown card id: " + id);
    return it->second.typeName;
}

std::shared_ptr<Card> Registry::createCard(const std::string& id,
                                            const std::string& cardTypeName) const {
    auto card = std::make_shared<Card>(id, cardType(cardTypeName));
    // Apply definition attributes if card id is known
    auto it = m_cardDefinitions.find(id);
    if (it != m_cardDefinitions.end()) {
        for (const auto& [key, val] : it->second.attributes)
            card->setAttribute(key, val);
    }
    return card;
}

std::unique_ptr<Deck> Registry::createDeck(const std::string& name,
                                            const std::string& deckTypeName) const {
    return std::make_unique<Deck>(name, deckType(deckTypeName));
}

} // namespace engine
