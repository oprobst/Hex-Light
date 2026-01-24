#pragma once

#include "../Effect.h"

class ChaseCometEffect : public Effect {
private:
    unsigned long lastMove = 0;
    uint8_t currentHex = 0;
    uint8_t logicalPosition = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long moveInterval = 105 - ctx.speed;

        if (now - lastMove >= moveInterval) {
            lastMove = now;
            logicalPosition++;
            if (logicalPosition >= ctx.ledsPerHexagon) {
                logicalPosition = 0;
                currentHex = (currentHex + 1) % ctx.numHexagons;
            }
        }

        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);

        int prevHex = (currentHex - 1 + ctx.numHexagons) % ctx.numHexagons;

        // Current hexagon: LEDs from 0 to logicalPosition
        for (int i = 0; i <= logicalPosition; i++) {
            uint8_t physicalPos = ctx.getHarmonizedLED(currentHex, i, false);
            uint8_t brightness = 255 - ((logicalPosition - i) * 7);
            CRGB color = ctx.color;
            color.nscale8(brightness);
            ctx.leds[ctx.hexagons[currentHex].startLED + physicalPos] = color;
        }

        // Previous hexagon: tail
        int remaining = ctx.ledsPerHexagon - 1 - logicalPosition;
        for (int i = 0; i < remaining && i < ctx.ledsPerHexagon; i++) {
            int pos = ctx.ledsPerHexagon - 1 - i;
            uint8_t physicalPos = ctx.getHarmonizedLED(prevHex, pos, false);
            uint8_t brightness = 255 - ((logicalPosition + 1 + i) * 7);
            if (brightness > 250) brightness = 0;
            CRGB color = ctx.color;
            color.nscale8(brightness);
            ctx.leds[ctx.hexagons[prevHex].startLED + physicalPos] = color;
        }
    }

    const char* getName() const override {
        return "Chase/Comet";
    }

    void reset() override {
        lastMove = 0;
        currentHex = 0;
        logicalPosition = 0;
    }
};
