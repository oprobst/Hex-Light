#pragma once

#include "../Effect.h"

class FireEffect : public Effect {
private:
    unsigned long lastUpdate = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 105 - ctx.speed;

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;

            for (int i = 0; i < ctx.numLeds; i++) {
                float baseHeat = (1.0f - ctx.ledGlobalY[i]) * 200;
                int heat = (int)baseHeat + random(-30, 30);
                heat = constrain(heat, 0, 255);

                if (heat < 85) {
                    ctx.leds[i] = CRGB(heat * 3, 0, 0);
                } else if (heat < 170) {
                    ctx.leds[i] = CRGB(255, (heat - 85) * 3, 0);
                } else {
                    ctx.leds[i] = CRGB(255, 255, (heat - 170) * 3);
                }
            }
        }
    }

    const char* getName() const override {
        return "Feuer";
    }

    void reset() override {
        lastUpdate = 0;
    }
};
