#pragma once

#include "../Effect.h"

class StrobeEffect : public Effect {
private:
    unsigned long lastStrobe = 0;
    bool strobeState = false;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long strobeInterval = 210 - (ctx.speed * 2);

        if (now - lastStrobe >= strobeInterval) {
            lastStrobe = now;
            strobeState = !strobeState;
        }

        if (strobeState) {
            fill_solid(ctx.leds, ctx.numLeds, ctx.color);
        } else {
            fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);
        }
    }

    const char* getName() const override {
        return "Strobe";
    }

    void reset() override {
        lastStrobe = 0;
        strobeState = false;
    }
};
