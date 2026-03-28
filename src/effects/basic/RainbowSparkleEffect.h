#pragma once

#include "../Effect.h"

class RainbowSparkleEffect : public Effect {
private:
    unsigned long lastSparkle = 0;
    uint8_t sparkleDecay[300] = {0};
    uint8_t sparkleHue[300] = {0};

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

        // Add new sparkles with random hues
        if (now - lastSparkle >= sparkleInterval) {
            lastSparkle = now;

            int numSparks = random(3, 9);
            for (int s = 0; s < numSparks; s++) {
                int randomLED = random(ctx.numLeds);
                sparkleDecay[randomLED] = 255;
                sparkleHue[randomLED] = random(256);
            }
        }

        // Set LEDs
        for (int i = 0; i < ctx.numLeds; i++) {
            if (sparkleDecay[i] > 0) {
                CRGB color = CHSV(sparkleHue[i], 255, sparkleDecay[i]);
                ctx.leds[i] = color;
            } else {
                ctx.leds[i] = CRGB::Black;
            }
        }
    }

    const char* getName() const override {
        return "Bunte Funken";
    }

    void reset() override {
        lastSparkle = 0;
        memset(sparkleDecay, 0, sizeof(sparkleDecay));
        memset(sparkleHue, 0, sizeof(sparkleHue));
    }
};
