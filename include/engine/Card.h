#pragma once

#include "CardAttribute.h"
#include "CardType.h"
#include <string>
#include <unordered_map>
#include <memory>

namespace engine {

// A concrete card instance with its own attribute values
class Card {
public:
    Card(std::string id, const CardType& type);

    const std::string& id() const { return m_id; }
    const CardType& type() const { return *m_type; }

    void setAttribute(const std::string& key, AttributeValue value);
    const AttributeValue& getAttribute(const std::string& key) const;
    bool hasAttribute(const std::string& key) const;

private:
    std::string m_id;
    const CardType* m_type;
    std::unordered_map<std::string, AttributeValue> m_attributes;
};

} // namespace engine
