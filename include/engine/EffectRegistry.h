#pragma once

#include "Effect.h"
#include <unordered_map>
#include <string>
#include <stdexcept>

namespace engine {

class EffectRegistry {
public:
    void registerEffect(const std::string& typeName, EffectFactory factory);
    bool has(const std::string& typeName) const;

    // Instantiate an EffectFn from a definition
    EffectFn create(const EffectDefinition& def) const;

    // Register all built-in effects (deal_damage, draw_cards, gain_resource, lose_resource)
    void registerBuiltins();

private:
    std::unordered_map<std::string, EffectFactory> m_factories;
};

// ---------------------------------------------------------------------------
// Built-in effect factories
// ---------------------------------------------------------------------------
namespace Effects {

    // params: target ("self" | "opponent"), amount ("N" or "attr:key")
    EffectFactory dealDamage();

    // params: count ("N")
    EffectFactory drawCards();

    // params: resource (name), amount ("N" or "attr:key")
    EffectFactory gainResource();

    // params: resource (name), amount ("N" or "attr:key")
    EffectFactory loseResource();

} // namespace Effects
} // namespace engine
