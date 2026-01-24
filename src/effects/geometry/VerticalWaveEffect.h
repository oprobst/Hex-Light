#pragma once

#include "../Effect.h"

class VerticalWaveEffect : public Effect {
private:
    float wavePosition = 0.0f;
    unsigned long lastMove = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = 105 - ctx.speed;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            wavePosition += 0.02f;
            if (wavePosition > 1.5f) wavePosition = -0.5f;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            float dist = fabs(ctx.ledGlobalY[i] - wavePosition);

            if (dist < 0.15f) {
                uint8_t brightness = (uint8_t)((1.0f - dist / 0.15f) * 255);
                CRGB color = ctx.color;
                color.nscale8(brightness);
                ctx.leds[i] = color;
            } else {
                ctx.leds[i] = CRGB::Black;
            }
        }
    }

    const char* getName() const override {
        return "Welle (Kanten)";
    }

    void reset() override {
        wavePosition = 0.0f;
        lastMove = 0;
    }
};
