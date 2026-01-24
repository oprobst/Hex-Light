#pragma once

#include "../Effect.h"

class VerticalGradientEffect : public Effect {
public:
    void update(EffectContext& ctx) override {
        for (int i = 0; i < ctx.numLeds; i++) {
            CRGB color = ctx.color;
            color.nscale8((uint8_t)(ctx.ledGlobalY[i] * 255));
            ctx.leds[i] = color;
        }
    }

    const char* getName() const override {
        return "V-Farbverlauf";
    }
};
