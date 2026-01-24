#pragma once

#include "../Effect.h"

class VerticalRainbowEffect : public Effect {
private:
    uint8_t hueOffset = 0;
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = max(5UL, 105UL - ctx.speed);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;
            hueOffset += 2;
        }

        for (int i = 0; i < ctx.numLeds; i++) {
            uint8_t hue = (uint8_t)(ctx.ledGlobalY[i] * 255) + hueOffset;
            ctx.leds[i] = CHSV(hue, 255, 255);
        }
    }

    const char* getName() const override {
        return "V-Regenbogen";
    }

    void reset() override {
        hueOffset = 0;
        lastUpdate = 0;
    }
};
