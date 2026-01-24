#pragma once

#include "../Effect.h"

class PhysicalCircleEffect : public Effect {
private:
    unsigned long lastMove = 0;
    uint8_t physicalPosition = 0;
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
                physicalPosition = (physicalPosition + 1) % ctx.ledsPerHexagon;

                if (physicalPosition == 0) {
                    pauseBeforeStart = true;
                }
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        if (!pauseBeforeStart) {
            for (int i = 0; i < ctx.numHexagons; i++) {
                if (ctx.hexagons[i].enabled) {
                    ctx.leds[ctx.hexagons[i].startLED + physicalPosition] = ctx.color;
                }
            }
        }
    }

    const char* getName() const override {
        return "Phys. Kreislauf";
    }

    void reset() override {
        lastMove = 0;
        physicalPosition = 0;
        pauseBeforeStart = false;
    }
};
