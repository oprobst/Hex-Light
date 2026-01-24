#pragma once

#include "../Effect.h"

class RainEffect : public Effect {
private:
    uint8_t dropBrightness[300] = {0};
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 80 - (ctx.speed * 0.7f);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;

            // Fade drops
            for (int h = 0; h < ctx.numHexagons; h++) {
                if (!ctx.hexagons[h].enabled) continue;

                for (int j = 0; j < ctx.hexagons[h].count; j++) {
                    uint16_t ledIndex = ctx.hexagons[h].startLED + j;
                    if (dropBrightness[ledIndex] > 0) {
                        dropBrightness[ledIndex] -= 15;
                        if (dropBrightness[ledIndex] < 15) dropBrightness[ledIndex] = 0;
                    }
                }
            }

            // Spawn new drops at top
            if (random(100) < 30 + ctx.speed / 3) {
                for (int h = 0; h < ctx.numHexagons; h++) {
                    if (!ctx.hexagons[h].enabled) continue;
                    for (int j = 0; j < ctx.hexagons[h].count; j++) {
                        float y = ctx.getLEDPreciseY(h, j);
                        if (y > 0.9f && random(100) < 20) {
                            dropBrightness[ctx.hexagons[h].startLED + j] = 255;
                        }
                    }
                }
            }
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            if (dropBrightness[i] > 0) {
                CRGB color = ctx.color;
                color.nscale8(dropBrightness[i]);
                ctx.leds[i] = color;
            } else {
                ctx.leds[i] = CRGB::Black;
            }
        }
    }

    const char* getName() const override {
        return "Regen";
    }

    void reset() override {
        memset(dropBrightness, 0, sizeof(dropBrightness));
        lastUpdate = 0;
    }
};
