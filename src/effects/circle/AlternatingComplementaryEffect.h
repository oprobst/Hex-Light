#pragma once

#include "../Effect.h"

class AlternatingComplementaryEffect : public Effect {
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

        CHSV hsvColor = rgb2hsv_approximate(ctx.color);
        hsvColor.hue += 128;
        CRGB complementaryColor = hsvColor;

        uint8_t oppositeLogical = (logicalPosition + (ctx.ledsPerHexagon / 2)) % ctx.ledsPerHexagon;

        for (int i = 0; i < ctx.numHexagons; i++) {
            if (ctx.hexagons[i].enabled) {
                bool reverseDirection = (i % 2 == 1);
                uint8_t physicalPos = ctx.getHarmonizedLED(i, logicalPosition, reverseDirection);
                uint8_t physicalOpp = ctx.getHarmonizedLED(i, oppositeLogical, reverseDirection);

                ctx.leds[ctx.hexagons[i].startLED + physicalPos] = ctx.color;
                ctx.leds[ctx.hexagons[i].startLED + physicalOpp] = complementaryColor;
            }
        }
    }

    const char* getName() const override {
        return "Alt.+Kompl.";
    }

    void reset() override {
        lastMove = 0;
        logicalPosition = 0;
    }
};
