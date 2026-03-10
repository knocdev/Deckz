#include "engine/EffectRegistry.h"
#include "engine/EffectContext.h"
#include "engine/GameState.h"
#include "engine/CardType.h"
#include <stdexcept>

namespace engine {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static int resolveInt(const std::string& val, const Card* card) {
    if (val.rfind("attr:", 0) == 0) {
        const std::string key = val.substr(5);
        if (!card)
            throw std::runtime_error("Effect references card attribute '" + key + "' but no card in context");
        return std::get<int>(card->getAttribute(key));
    }
    return std::stoi(val);
}

// ---------------------------------------------------------------------------
// EffectRegistry
// ---------------------------------------------------------------------------

void EffectRegistry::registerEffect(const std::string& typeName, EffectFactory factory) {
    m_factories[typeName] = std::move(factory);
}

bool EffectRegistry::has(const std::string& typeName) const {
    return m_factories.count(typeName) > 0;
}

EffectFn EffectRegistry::create(const EffectDefinition& def) const {
    auto it = m_factories.find(def.typeName);
    if (it == m_factories.end())
        throw std::out_of_range("Unknown effect type: " + def.typeName);
    return it->second(def.params);
}

void EffectRegistry::registerBuiltins() {
    registerEffect("deal_damage",   Effects::dealDamage());
    registerEffect("draw_cards",    Effects::drawCards());
    registerEffect("gain_resource", Effects::gainResource());
    registerEffect("lose_resource", Effects::loseResource());
}

// ---------------------------------------------------------------------------
// Built-in effect factories
// ---------------------------------------------------------------------------

namespace Effects {

EffectFactory dealDamage() {
    return [](const EffectParams& params) -> EffectFn {
        std::string target = params.count("target") ? params.at("target") : "opponent";
        std::string amountStr = params.at("amount");
        return [target, amountStr](EffectContext& ctx) {
            int amount = resolveInt(amountStr, ctx.card);
            Player* t = (target == "self") ? ctx.source : ctx.opponent;
            if (!t) throw std::runtime_error("deal_damage: no target player");
            int hp = t->hasResource("health") ? t->getResource("health") : 0;
            t->setResource("health", hp - amount);
        };
    };
}

EffectFactory drawCards() {
    return [](const EffectParams& params) -> EffectFn {
        std::string countStr = params.count("count") ? params.at("count") : "1";
        return [countStr](EffectContext& ctx) {
            int count = resolveInt(countStr, ctx.card);
            for (int i = 0; i < count; ++i) {
                if (ctx.source->deck().empty()) break;
                ctx.source->hand().addCard(ctx.source->deck().drawTop());
            }
        };
    };
}

EffectFactory gainResource() {
    return [](const EffectParams& params) -> EffectFn {
        std::string resource  = params.at("resource");
        std::string amountStr = params.at("amount");
        return [resource, amountStr](EffectContext& ctx) {
            int amount  = resolveInt(amountStr, ctx.card);
            int current = ctx.source->hasResource(resource) ? ctx.source->getResource(resource) : 0;
            ctx.source->setResource(resource, current + amount);
        };
    };
}

EffectFactory loseResource() {
    return [](const EffectParams& params) -> EffectFn {
        std::string resource  = params.at("resource");
        std::string amountStr = params.at("amount");
        return [resource, amountStr](EffectContext& ctx) {
            int amount  = resolveInt(amountStr, ctx.card);
            int current = ctx.source->hasResource(resource) ? ctx.source->getResource(resource) : 0;
            ctx.source->setResource(resource, current - amount);
        };
    };
}

} // namespace Effects
} // namespace engine
