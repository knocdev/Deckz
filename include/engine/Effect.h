#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <stdexcept>

namespace engine {

struct EffectContext; // defined in EffectContext.h

enum class EffectTrigger { OnPlay, OnTurnStart, OnTurnEnd, OnDraw, Custom };

inline EffectTrigger effectTriggerFromString(const std::string& s) {
    if (s == "on_play")       return EffectTrigger::OnPlay;
    if (s == "on_turn_start") return EffectTrigger::OnTurnStart;
    if (s == "on_turn_end")   return EffectTrigger::OnTurnEnd;
    if (s == "on_draw")       return EffectTrigger::OnDraw;
    if (s == "custom")        return EffectTrigger::Custom;
    throw std::invalid_argument("Unknown effect trigger: " + s);
}

using EffectParams  = std::unordered_map<std::string, std::string>;
using EffectFn      = std::function<void(EffectContext&)>;
using EffectFactory = std::function<EffectFn(const EffectParams&)>;

struct EffectDefinition {
    std::string   typeName;
    EffectTrigger trigger;
    EffectParams  params;
};

} // namespace engine
