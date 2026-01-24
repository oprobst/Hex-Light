#pragma once

#include "../Effect.h"
#include <cmath>

class ColorWheelEffect : public Effect {
private:
    float rotation = 0.0f;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = max(5UL, 50UL - (ctx.speed / 2));

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            rotation += 0.05f;
            if (rotation > 6.28f) rotation -= 6.28f;
        }

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
                float angle = atan2(dy, dx) + rotation;

                uint8_t hue = (uint8_t)((angle + 3.14159f) / 6.28318f * 255);
                ctx.leds[ledIndex] = CHSV(hue, 255, 255);
            }
        }
    }

    const char* getName() const override {
        return "Farbrad";
    }

    void reset() override {
        rotation = 0.0f;
        lastUpdate = 0;
    }
};
