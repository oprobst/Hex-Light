#pragma once

#include "../Effect.h"

class SparkleEffect : public Effect {
private:
    unsigned long lastSparkle = 0;
    uint8_t sparkleDecay[300] = {0};  // Max LEDs

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long sparkleInterval = 150 - ctx.speed;

        // Fade all LEDs
        for (int i = 0; i < ctx.numLeds; i++) {
            if (sparkleDecay[i] > 5) {
                sparkleDecay[i] -= 5;
            } else {
                sparkleDecay[i] = 0;
            }
        }

        // Add new sparkles
        if (now - lastSparkle >= sparkleInterval) {
            lastSparkle = now;

            int numSparks = random(3, 9);
            for (int s = 0; s < numSparks; s++) {
                int randomLED = random(ctx.numLeds);
                sparkleDecay[randomLED] = 255;
            }
        }

        // Set LEDs
        for (int i = 0; i < ctx.numLeds; i++) {
            if (sparkleDecay[i] > 0) {
                CRGB color = ctx.color;
                color.nscale8(sparkleDecay[i]);
                ctx.leds[i] = color;
            } else {
                ctx.leds[i] = CRGB::Black;
            }
        }
    }

    const char* getName() const override {
        return "Funken";
    }

    void reset() override {
        lastSparkle = 0;
        memset(sparkleDecay, 0, sizeof(sparkleDecay));
    }
};
