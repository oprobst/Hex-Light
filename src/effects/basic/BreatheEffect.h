#pragma once

#include "../Effect.h"

class BreatheEffect : public Effect {
private:
    uint8_t breathValue = 128;
    int8_t breathDirection = 1;
    uint8_t frameCounter = 0;

public:
    void update(EffectContext& ctx) override {
        frameCounter++;
        uint8_t speedDivisor = max(1, (101 - ctx.speed) / 2);

        if (frameCounter >= speedDivisor) {
            frameCounter = 0;

            breathValue += breathDirection;
            if (breathValue >= 255) {
                breathValue = 255;
                breathDirection = -1;
            } else if (breathValue <= 128) {
                breathValue = 128;
                breathDirection = 1;
            }
        }

        CRGB scaledColor = ctx.color;
        scaledColor.nscale8(breathValue);
        fill_solid(ctx.leds, ctx.numLeds, scaledColor);
    }

    const char* getName() const override {
        return "Breathe";
    }

    void reset() override {
        breathValue = 128;
        breathDirection = 1;
        frameCounter = 0;
    }
};
