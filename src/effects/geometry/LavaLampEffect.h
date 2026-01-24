#pragma once

#include "../Effect.h"
#include <cmath>

class LavaLampEffect : public Effect {
private:
    float time = 0.0f;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = max(10UL, 60UL - (ctx.speed / 2));

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            time += 0.02f;
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

                float blob1 = sin(y * 3.0f + time * 0.5f) * sin(x * 2.0f + time * 0.3f);
                float blob2 = sin(y * 2.0f - time * 0.4f + 1.5f) * cos(x * 3.0f + time * 0.2f);
                float blob3 = cos(y * 4.0f + time * 0.6f) * sin(x * 1.5f - time * 0.35f);

                float combined = (blob1 + blob2 + blob3) / 3.0f;
                combined = (combined + 1.0f) / 2.0f;

                CHSV hsvColor = rgb2hsv_approximate(ctx.color);
                hsvColor.hue += (uint8_t)(combined * 30 - 15);
                hsvColor.val = (uint8_t)(combined * 255);

                ctx.leds[ledIndex] = hsvColor;
            }
        }
    }

    const char* getName() const override {
        return "Lava Lamp";
    }

    void reset() override {
        time = 0.0f;
        lastUpdate = 0;
    }
};
