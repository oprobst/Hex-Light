#pragma once

#include "../Effect.h"
#include <cmath>

class BreathingCenterEffect : public Effect {
private:
    float breathPhase = 0.0f;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = max(10UL, 50UL - (ctx.speed / 2));

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            breathPhase += 0.05f;
            if (breathPhase > 6.28f) breathPhase -= 6.28f;
        }

        float breathRadius = (sin(breathPhase) + 1.0f) / 2.0f * 2.0f;

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            for (int j = 0; j < ctx.hexagons[h].count; j++) {
                uint16_t ledIndex = ctx.hexagons[h].startLED + j;

                float x = ctx.ledGlobalX[ledIndex];
                float y = ctx.getLEDPreciseY(h, j);
                if (ctx.geometryLoaded && h < ctx.numConfiguredHexagons) {
                    y = ctx.hexGeometry[h].globalY + (y - 0.5f);
                }

                float dx = x - ctx.centerX;
                float dy = y - ctx.centerY;
                float dist = sqrt(dx * dx + dy * dy);

                float brightness = 1.0f - (dist / breathRadius);
                brightness = constrain(brightness, 0.0f, 1.0f);

                CRGB color = ctx.color;
                color.nscale8((uint8_t)(brightness * 255));
                ctx.leds[ledIndex] = color;
            }
        }
    }

    const char* getName() const override {
        return "Atmendes Zentrum";
    }

    void reset() override {
        breathPhase = 0.0f;
        lastUpdate = 0;
    }
};
