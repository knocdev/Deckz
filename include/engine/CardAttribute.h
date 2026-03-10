#pragma once

#include <string>
#include <variant>
#include <stdexcept>

namespace engine {

// The set of value types an attribute can hold
using AttributeValue = std::variant<int, float, bool, std::string>;

enum class AttributeType { Int, Float, Bool, String };

inline AttributeType attributeTypeFromString(const std::string& s) {
    if (s == "int")    return AttributeType::Int;
    if (s == "float")  return AttributeType::Float;
    if (s == "bool")   return AttributeType::Bool;
    if (s == "string") return AttributeType::String;
    throw std::invalid_argument("Unknown attribute type: " + s);
}

inline std::string attributeTypeToString(AttributeType t) {
    switch (t) {
        case AttributeType::Int:    return "int";
        case AttributeType::Float:  return "float";
        case AttributeType::Bool:   return "bool";
        case AttributeType::String: return "string";
    }
    return "unknown";
}

} // namespace engine
