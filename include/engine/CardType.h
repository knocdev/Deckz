#pragma once

#include "CardAttribute.h"
#include "Effect.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace engine {

struct AttributeSchema {
    AttributeType type;
    AttributeValue defaultValue;
};

// Defines the template for a category of cards (e.g. "Creature", "Spell")
class CardType {
public:
    explicit CardType(std::string name);

    const std::string& name() const { return m_name; }
    const std::unordered_map<std::string, AttributeSchema>& schema() const { return m_schema; }

    void addAttribute(const std::string& key, AttributeType type, AttributeValue defaultValue);
    bool hasAttribute(const std::string& key) const;
    const AttributeSchema& getAttribute(const std::string& key) const;

    void addEffect(EffectDefinition effect);
    const std::vector<EffectDefinition>& effects() const { return m_effects; }

private:
    std::string m_name;
    std::unordered_map<std::string, AttributeSchema> m_schema;
    std::vector<EffectDefinition> m_effects;
};

} // namespace engine
