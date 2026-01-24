#pragma once

#include "../Effect.h"

class StrobeLongEffect : public Effect {
private:
    unsigned long lastChange = 0;
    bool strobeState = false;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long onInterval = 55 - (ctx.speed / 2);
        unsigned long offInterval = onInterval * 9;

        unsigned long changeInterval = strobeState ? onInterval : offInterval;

        if (now - lastChange >= changeInterval) {
            lastChange = now;
            strobeState = !strobeState;
        }

        if (strobeState) {
            fill_solid(ctx.leds, ctx.numLeds, ctx.color);
        } else {
            fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);
        }
    }

    const char* getName() const override {
        return "Strobe Long";
    }

    void reset() override {
        lastChange = 0;
        strobeState = false;
    }
};
