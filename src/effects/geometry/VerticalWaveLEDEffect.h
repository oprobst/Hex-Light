#pragma once

#include "../Effect.h"

class VerticalWaveLEDEffect : public Effect {
private:
    float wavePosition = 0.0f;
    unsigned long lastMove = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = 105 - ctx.speed;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            wavePosition += 0.025f;
            // Kürzerer Range für weniger Pause: -0.15 bis 1.15
            if (wavePosition > 1.15f) wavePosition = -0.15f;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            // Verwende die bereits normalisierten globalen Y-Koordinaten (0.0-1.0)
            float y = ctx.ledGlobalY[i];

            float dist = fabs(y - wavePosition);

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
        return "Welle (LED)";
    }

    void reset() override {
        wavePosition = 0.0f;
        lastMove = 0;
    }
};
