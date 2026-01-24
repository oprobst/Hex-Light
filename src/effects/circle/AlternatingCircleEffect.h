#pragma once

#include "../Effect.h"

class AlternatingCircleEffect : public Effect {
private:
    unsigned long lastMove = 0;
    uint8_t logicalPosition = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = (105 - ctx.speed) / 2;
        if (moveInterval < 1) moveInterval = 1;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            logicalPosition = (logicalPosition + 1) % ctx.ledsPerHexagon;
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        for (int i = 0; i < ctx.numHexagons; i++) {
            if (ctx.hexagons[i].enabled) {
                bool reverseDirection = (i % 2 == 1);
                uint8_t physicalPos = ctx.getHarmonizedLED(i, logicalPosition, reverseDirection);
                ctx.leds[ctx.hexagons[i].startLED + physicalPos] = ctx.color;
            }
        }
    }

    const char* getName() const override {
        return "Alternierend";
    }

    void reset() override {
        lastMove = 0;
        logicalPosition = 0;
    }
};
