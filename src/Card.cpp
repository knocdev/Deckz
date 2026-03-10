#include "engine/Card.h"
#include "engine/CardType.h"
#include <stdexcept>

namespace engine {

Card::Card(std::string id, const CardType& type)
    : m_id(std::move(id)), m_type(&type)
{
    // Populate attributes with their schema defaults
    for (const auto& [key, schema] : type.schema())
        m_attributes[key] = schema.defaultValue;
}

void Card::setAttribute(const std::string& key, AttributeValue value) {
    if (!m_type->hasAttribute(key))
        throw std::invalid_argument("CardType '" + m_type->name() + "' has no attribute '" + key + "'");
    m_attributes[key] = std::move(value);
}

const AttributeValue& Card::getAttribute(const std::string& key) const {
    auto it = m_attributes.find(key);
    if (it == m_attributes.end())
        throw std::out_of_range("Card '" + m_id + "' has no attribute '" + key + "'");
    return it->second;
}

bool Card::hasAttribute(const std::string& key) const {
    return m_attributes.count(key) > 0;
}

} // namespace engine
