#pragma once

#include "../Effect.h"
#include <cmath>

class WaveWallEffect : public Effect {
private:
    float phase = 0.0f;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = max(10UL, 40UL - (ctx.speed / 3));

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            phase += 0.1f;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            float x = ctx.ledGlobalX[i];
            float y = ctx.ledGlobalY[i];

            // Mehrere überlagerte Sinuswellen für Wassereffekt
            float wave = sin(x * 4.0f + phase) * 0.15f;
            wave += sin(x * 7.0f - phase * 0.7f) * 0.08f;
            wave += sin(x * 2.0f + phase * 1.3f) * 0.1f;

            float baseY = 0.5f + wave;
            float dist = fabs(y - baseY);

            if (dist < 0.12f) {
                uint8_t brightness = (uint8_t)((1.0f - dist / 0.12f) * 255);
                CRGB color = ctx.color;
                color.nscale8(brightness);
                ctx.leds[i] = color;
            } else {
                ctx.leds[i] = CRGB::Black;
            }
        }
    }

    const char* getName() const override {
        return "Wellenwand";
    }

    void reset() override {
        phase = 0.0f;
        lastUpdate = 0;
    }
};
