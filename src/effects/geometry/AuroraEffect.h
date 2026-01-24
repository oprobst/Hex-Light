#pragma once

#include "../Effect.h"
#include <cmath>

class AuroraEffect : public Effect {
private:
    float phase = 0.0f;
    uint8_t frameCounter = 0;

public:
    void update(EffectContext& ctx) override {
        frameCounter++;
        if (frameCounter >= (101 - ctx.speed) / 2) {
            phase += 0.05f;
            frameCounter = 0;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            float wave1 = sin(ctx.ledGlobalY[i] * 6.28f + phase) * 0.5f + 0.5f;
            float wave2 = sin(ctx.ledGlobalX[i] * 4.0f + phase * 1.3f) * 0.5f + 0.5f;
            float wave3 = sin((ctx.ledGlobalY[i] + ctx.ledGlobalX[i]) * 3.0f + phase * 0.7f) * 0.5f + 0.5f;

            uint8_t hue = 96 + (uint8_t)((wave1 + wave2) * 64);
            uint8_t brightness = (uint8_t)(wave3 * 200 + 55);

            ctx.leds[i] = CHSV(hue, 255, brightness);
        }
    }

    const char* getName() const override {
        return "Aurora";
    }

    void reset() override {
        phase = 0.0f;
        frameCounter = 0;
    }
};
