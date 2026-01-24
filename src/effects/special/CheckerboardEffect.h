#pragma once

#include "../Effect.h"

class CheckerboardEffect : public Effect {
private:
    bool phase = false;
    unsigned long lastToggle = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long toggleInterval = 1000 - (ctx.speed * 9);

        if (now - lastToggle >= toggleInterval) {
            lastToggle = now;
            phase = !phase;
        }

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            bool isOn = (h % 2 == 0) ? phase : !phase;

            for (int j = 0; j < ctx.hexagons[h].count; j++) {
                if (isOn) {
                    ctx.leds[ctx.hexagons[h].startLED + j] = ctx.color;
                } else {
                    ctx.leds[ctx.hexagons[h].startLED + j] = CRGB::Black;
                }
            }
        }
    }

    const char* getName() const override {
        return "Schachbrett";
    }

    void reset() override {
        phase = false;
        lastToggle = 0;
    }
};
