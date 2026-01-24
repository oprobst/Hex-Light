#pragma once

#include "../Effect.h"
#include <cmath>

class VortexEffect : public Effect {
private:
    float angle = 0.0f;
    unsigned long lastMove = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = max(5UL, 105UL - ctx.speed);

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            angle += 0.08f;
            if (angle > 6.28f) angle -= 6.28f;
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
                float ledAngle = atan2(dy, dx);

                float spiralValue = sin(ledAngle * 3.0f + distFromCenter * 4.0f + angle);

                if (spiralValue > 0.3f) {
                    uint8_t brightness = (uint8_t)((spiralValue - 0.3f) / 0.7f * 255);
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
        return "Strudel";
    }

    void reset() override {
        angle = 0.0f;
        lastMove = 0;
    }
};
