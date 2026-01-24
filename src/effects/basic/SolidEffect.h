#pragma once

#include "../Effect.h"

class SolidEffect : public Effect {
public:
    void update(EffectContext& ctx) override {
        fill_solid(ctx.leds, ctx.numLeds, ctx.color);
    }

    const char* getName() const override {
        return "Solid Color";
    }
};
