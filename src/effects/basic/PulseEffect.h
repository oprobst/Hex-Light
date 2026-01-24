#pragma once

#include "../Effect.h"

class PulseEffect : public Effect {
private:
    uint8_t pulseValue = 0;
    int8_t pulseDirection = 1;

public:
    void update(EffectContext& ctx) override {
        uint8_t stepSize = max(1, ctx.speed / 20);

        pulseValue += pulseDirection * stepSize;
        if (pulseValue >= 255 || pulseValue <= 0) {
            pulseDirection *= -1;
            pulseValue = constrain(pulseValue, 0, 255);
        }

        CRGB color = ctx.color;
        color.nscale8(pulseValue);
        fill_solid(ctx.leds, ctx.numLeds, color);
    }

    const char* getName() const override {
        return "Pulse";
    }

    void reset() override {
        pulseValue = 0;
        pulseDirection = 1;
    }
};
