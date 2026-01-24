#pragma once

#include "../Effect.h"

class RunningLightEffect : public Effect {
private:
    unsigned long lastMove = 0;
    uint8_t currentHex = 0;
    uint8_t logicalPosition = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = (105 - ctx.speed) / 2;
        if (moveInterval < 1) moveInterval = 1;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            logicalPosition++;
            if (logicalPosition >= ctx.ledsPerHexagon) {
                logicalPosition = 0;
                currentHex = (currentHex + 1) % ctx.numHexagons;
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        if (ctx.hexagons[currentHex].enabled) {
            uint8_t physicalPos = ctx.getHarmonizedLED(currentHex, logicalPosition, false);
            ctx.leds[ctx.hexagons[currentHex].startLED + physicalPos] = ctx.color;
        }
    }

    const char* getName() const override {
        return "Running Light";
    }

    void reset() override {
        lastMove = 0;
        currentHex = 0;
        logicalPosition = 0;
    }
};
