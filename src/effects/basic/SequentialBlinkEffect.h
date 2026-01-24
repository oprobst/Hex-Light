#pragma once

#include "../Effect.h"

class SequentialBlinkEffect : public Effect {
private:
    unsigned long lastChange = 0;
    int currentHexagon = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long changeInterval = 1050 - (ctx.speed * 10);

        if (now - lastChange >= changeInterval) {
            lastChange = now;
            currentHexagon = (currentHexagon + 1) % ctx.numHexagons;
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        if (ctx.hexagons[currentHexagon].enabled) {
            for (int j = 0; j < ctx.hexagons[currentHexagon].count; j++) {
                ctx.leds[ctx.hexagons[currentHexagon].startLED + j] = ctx.color;
            }
        }
    }

    const char* getName() const override {
        return "Blink Seq.";
    }

    void reset() override {
        lastChange = 0;
        currentHexagon = 0;
    }
};
