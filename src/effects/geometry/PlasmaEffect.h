#pragma once

#include "../Effect.h"
#include <cmath>

class PlasmaEffect : public Effect {
private:
    float time = 0.0f;
    uint8_t frameCounter = 0;

public:
    void update(EffectContext& ctx) override {
        frameCounter++;
        if (frameCounter >= (101 - ctx.speed) / 3) {
            time += 0.1f;
            frameCounter = 0;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            float x = ctx.ledGlobalX[i];
            float y = ctx.ledGlobalY[i];

            float v1 = sin(x * 10.0f + time);
            float v2 = sin(10.0f * (x * sin(time / 2.0f) + y * cos(time / 3.0f)) + time);
            float cx = x + 0.5f * sin(time / 5.0f);
            float cy = y + 0.5f * cos(time / 3.0f);
            float v3 = sin(sqrt(100.0f * (cx * cx + cy * cy) + 1.0f) + time);

            float v = (v1 + v2 + v3) / 3.0f;
            uint8_t hue = (uint8_t)((v + 1.0f) * 127.5f);
            ctx.leds[i] = CHSV(hue, 255, 255);
        }
    }

    const char* getName() const override {
        return "Plasma";
    }

    void reset() override {
        time = 0.0f;
        frameCounter = 0;
    }
};
