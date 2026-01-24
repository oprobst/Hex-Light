#pragma once

#include "../Effect.h"

class RainbowEffect : public Effect {
private:
    uint8_t hue = 0;
    uint8_t frameCounter = 0;

public:
    void update(EffectContext& ctx) override {
        fill_rainbow(ctx.leds, ctx.numLeds, hue, 7);

        frameCounter++;
        if (frameCounter >= (101 - ctx.speed)) {
            hue++;
            frameCounter = 0;
        }
    }

    const char* getName() const override {
        return "Rainbow";
    }

    void reset() override {
        hue = 0;
        frameCounter = 0;
    }
};
