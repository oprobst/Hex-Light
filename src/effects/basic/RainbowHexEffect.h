#pragma once

#include "../Effect.h"

class RainbowHexEffect : public Effect {
private:
    uint8_t hue = 0;
    uint8_t frameCounter = 0;

public:
    void update(EffectContext& ctx) override {
        uint8_t huePerHexagon = 256 / ctx.numHexagons;

        for (int i = 0; i < ctx.numHexagons; i++) {
            if (ctx.hexagons[i].enabled) {
                CRGB hexColor = CHSV(hue + (i * huePerHexagon), 255, 255);
                for (int j = 0; j < ctx.hexagons[i].count; j++) {
                    ctx.leds[ctx.hexagons[i].startLED + j] = hexColor;
                }
            }
        }

        frameCounter++;
        if (frameCounter >= (101 - ctx.speed)) {
            hue++;
            frameCounter = 0;
        }
    }

    const char* getName() const override {
        return "Rainbow/Hex";
    }

    void reset() override {
        hue = 0;
        frameCounter = 0;
    }
};
