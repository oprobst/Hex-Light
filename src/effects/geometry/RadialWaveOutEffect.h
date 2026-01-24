#pragma once

#include "../Effect.h"
#include <cmath>

class RadialWaveOutEffect : public Effect {
private:
    float waveRadius = 0.0f;
    unsigned long lastMove = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = 105 - ctx.speed;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            waveRadius += 0.04f;
            if (waveRadius > 3.0f) waveRadius = 0.0f;
        }

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            for (int j = 0; j < ctx.hexagons[h].count; j++) {
                uint16_t ledIndex = ctx.hexagons[h].startLED + j;

                float ledX = ctx.ledGlobalX[ledIndex];
                float ledY = ctx.getLEDPreciseY(h, j);
                if (ctx.geometryLoaded && h < ctx.numConfiguredHexagons) {
                    ledY = ctx.hexGeometry[h].globalY + (ledY - 0.5f);
                }

                float dx = ledX - ctx.centerX;
                float dy = ledY - ctx.centerY;
                float distFromCenter = sqrt(dx * dx + dy * dy);
                float distToWave = fabs(distFromCenter - waveRadius);

                if (distToWave < 0.15f) {
                    uint8_t brightness = (uint8_t)((1.0f - distToWave / 0.15f) * 255);
                    CRGB color = ctx.color;
                    color.nscale8(brightness);
                    ctx.leds[ledIndex] = color;
                } else {
                    ctx.leds[ledIndex] = CRGB::Black;
                }
            }
        }
    }

    const char* getName() const override {
        return "Radial In->Out";
    }

    void reset() override {
        waveRadius = 0.0f;
        lastMove = 0;
    }
};
