#include "engine/CardType.h"
#include <stdexcept>

namespace engine {

CardType::CardType(std::string name) : m_name(std::move(name)) {}

void CardType::addAttribute(const std::string& key, AttributeType type, AttributeValue defaultValue) {
    m_schema[key] = { type, std::move(defaultValue) };
}

bool CardType::hasAttribute(const std::string& key) const {
    return m_schema.count(key) > 0;
}

const AttributeSchema& CardType::getAttribute(const std::string& key) const {
    auto it = m_schema.find(key);
    if (it == m_schema.end())
        throw std::out_of_range("CardType '" + m_name + "' has no attribute '" + key + "'");
    return it->second;
}

void CardType::addEffect(EffectDefinition effect) {
    m_effects.push_back(std::move(effect));
}

} // namespace engine
