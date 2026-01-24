#pragma once

#include "../Effect.h"

class HorizontalRainbowEffect : public Effect {
private:
    uint8_t hueOffset = 0;
    unsigned long lastUpdate = 0;
    float minX = 999.0f, maxX = -999.0f;
    bool initialized = false;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();

        if (!initialized) {
            for (int i = 0; i < ctx.numLeds; i++) {
                if (ctx.ledGlobalX[i] < minX) minX = ctx.ledGlobalX[i];
                if (ctx.ledGlobalX[i] > maxX) maxX = ctx.ledGlobalX[i];
            }
            initialized = true;
        }
        float rangeX = maxX - minX;
        if (rangeX < 0.01f) rangeX = 1.0f;

        unsigned long updateInterval = max(5UL, 105UL - ctx.speed);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            hueOffset += 2;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            float normalizedX = (ctx.ledGlobalX[i] - minX) / rangeX;
            uint8_t hue = (uint8_t)(normalizedX * 255) + hueOffset;
            ctx.leds[i] = CHSV(hue, 255, 255);
        }
    }

    const char* getName() const override {
        return "H-Regenbogen";
    }

    void reset() override {
        hueOffset = 0;
        lastUpdate = 0;
        initialized = false;
        minX = 999.0f;
        maxX = -999.0f;
    }
};
