#pragma once

#include "../Effect.h"
#include <cmath>

class WaveCollisionEffect : public Effect {
private:
    float wave1Pos = 0.0f;
    float wave2Pos = 1.0f;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 80 - (ctx.speed * 0.7f);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            wave1Pos += 0.025f;
            wave2Pos -= 0.025f;

            // Kürzerer Range für weniger Pause
            if (wave1Pos > 1.15f) wave1Pos = -0.15f;
            if (wave2Pos < -0.15f) wave2Pos = 1.15f;
        }

        CHSV hsvColor = rgb2hsv_approximate(ctx.color);
        hsvColor.hue += 128;
        CRGB complementaryColor = hsvColor;

        for (int i = 0; i < ctx.numLeds; i++) {
            // Verwende die bereits normalisierten globalen Y-Koordinaten (0.0-1.0)
            float y = ctx.ledGlobalY[i];

            float dist1 = fabs(y - wave1Pos);
            float dist2 = fabs(y - wave2Pos);

            CRGB finalColor = CRGB::Black;

            if (dist1 < 0.12f) {
                uint8_t brightness = (uint8_t)((1.0f - dist1 / 0.12f) * 255);
                CRGB c = ctx.color;
                c.nscale8(brightness);
                finalColor += c;
            }
            if (dist2 < 0.12f) {
                uint8_t brightness = (uint8_t)((1.0f - dist2 / 0.12f) * 255);
                CRGB c = complementaryColor;
                c.nscale8(brightness);
                finalColor += c;
            }

            ctx.leds[i] = finalColor;
        }
    }

    const char* getName() const override {
        return "Wellen-Kollision";
    }

    void reset() override {
        wave1Pos = 0.0f;
        wave2Pos = 1.0f;
        lastUpdate = 0;
    }
};
