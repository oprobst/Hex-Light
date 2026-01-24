#pragma once

#include "../Effect.h"
#include <cmath>

class MeteorEffect : public Effect {
private:
    float meteorY[3] = {1.5f, 2.0f, 2.5f};
    float meteorX[3] = {0.0f, 0.5f, -0.3f};
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 50 - (ctx.speed / 3);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;

            for (int m = 0; m < 3; m++) {
                meteorY[m] -= 0.08f;
                meteorX[m] += 0.02f;

                if (meteorY[m] < -1.0f) {
                    meteorY[m] = 1.5f + random(100) / 100.0f;
                    meteorX[m] = -1.0f + random(200) / 100.0f;
                }
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            for (int j = 0; j < ctx.hexagons[h].count; j++) {
                uint16_t ledIndex = ctx.hexagons[h].startLED + j;

                float x = ctx.ledGlobalX[ledIndex];
                float y = ctx.getLEDPreciseY(h, j);
                if (ctx.geometryLoaded && h < ctx.numConfiguredHexagons) {
                    y = ctx.hexGeometry[h].globalY + (y - 0.5f);
                }

                for (int m = 0; m < 3; m++) {
                    float dx = x - meteorX[m];
                    float dy = y - meteorY[m];

                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist < 0.15f) {
                        uint8_t brightness = (uint8_t)((1.0f - dist / 0.15f) * 255);
                        CRGB color = CRGB::White;
                        color.nscale8(brightness);
                        ctx.leds[ledIndex] += color;
                    }

                    float tailDist = dy - dx * 0.5f;
                    if (tailDist > 0 && tailDist < 0.4f && fabs(dx) < 0.1f) {
                        uint8_t brightness = (uint8_t)((1.0f - tailDist / 0.4f) * 180);
                        CRGB color = ctx.color;
                        color.nscale8(brightness);
                        ctx.leds[ledIndex] += color;
                    }
                }
            }
        }
    }

    const char* getName() const override {
        return "Meteor";
    }

    void reset() override {
        meteorY[0] = 1.5f; meteorY[1] = 2.0f; meteorY[2] = 2.5f;
        meteorX[0] = 0.0f; meteorX[1] = 0.5f; meteorX[2] = -0.3f;
        lastUpdate = 0;
    }
};
