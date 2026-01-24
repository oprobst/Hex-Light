#pragma once

#include "../Effect.h"

class HarmonizedCircleEffect : public Effect {
private:
    unsigned long lastMove = 0;
    uint8_t logicalPosition = 0;
    bool pauseBeforeStart = false;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = (105 - ctx.speed) / 2;
        if (moveInterval < 1) moveInterval = 1;

        unsigned long currentInterval = pauseBeforeStart ? 1000 : moveInterval;

        if (now - lastMove >= currentInterval) {
            lastMove = now;

            if (pauseBeforeStart) {
                pauseBeforeStart = false;
            } else {
                logicalPosition = (logicalPosition + 1) % ctx.ledsPerHexagon;

                if (logicalPosition == 0) {
                    pauseBeforeStart = true;
                }
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        if (!pauseBeforeStart) {
            for (int i = 0; i < ctx.numHexagons; i++) {
                if (ctx.hexagons[i].enabled) {
                    uint8_t physicalPos = ctx.getHarmonizedLED(i, logicalPosition, false);
                    ctx.leds[ctx.hexagons[i].startLED + physicalPos] = ctx.color;
                }
            }
        }
    }

    const char* getName() const override {
        return "Harm. Kreislauf";
    }

    void reset() override {
        lastMove = 0;
        logicalPosition = 0;
        pauseBeforeStart = false;
    }
};
