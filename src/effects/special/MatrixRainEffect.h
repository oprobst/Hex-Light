#pragma once

#include "../Effect.h"
#include <cmath>

class MatrixRainEffect : public Effect {
private:
    uint8_t columnState[50] = {0};  // Max hexagons * 6
    uint8_t columnPos[50] = {0};
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 120 - ctx.speed;

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;

            int maxCols = ctx.numHexagons * 6;
            for (int col = 0; col < maxCols && col < 50; col++) {
                if (columnState[col] > 0) {
                    columnPos[col]++;
                    if (columnPos[col] > 6) {
                        columnState[col] = 0;
                    }
                } else if (random(100) < 15) {
                    columnState[col] = 255;
                    columnPos[col] = 0;
                }
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            for (int side = 0; side < 6; side++) {
                int colIdx = h * 6 + side;
                if (colIdx >= 50) continue;

                if (columnState[colIdx] > 0) {
                    for (int j = 0; j < ctx.hexagons[h].count; j++) {
                        float y = ctx.getLEDPreciseY(h, j);
                        float targetY = 1.0f - (columnPos[colIdx] / 6.0f);
                        float dist = fabs(y - targetY);

                        if (dist < 0.2f) {
                            uint8_t brightness = (uint8_t)((1.0f - dist / 0.2f) * 255);
                            CRGB color = ctx.color;
                            color.nscale8(brightness);
                            ctx.leds[ctx.hexagons[h].startLED + j] += color;
                        }
                    }
                }
            }
        }
    }

    const char* getName() const override {
        return "Matrix Rain";
    }

    void reset() override {
        memset(columnState, 0, sizeof(columnState));
        memset(columnPos, 0, sizeof(columnPos));
        lastUpdate = 0;
    }
};
