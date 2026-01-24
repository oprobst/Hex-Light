#pragma once

#include "../Effect.h"

class FlickerEffect : public Effect {
private:
    unsigned long lastFlicker = 0;
    unsigned long nextFlickerTime = 5000;
    bool isFlickering = false;
    uint8_t flickerStep = 0;
    unsigned long flickerStepTime = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();

        if (!isFlickering) {
            fill_solid(ctx.leds, ctx.numLeds, ctx.color);

            if (now - lastFlicker >= nextFlickerTime) {
                isFlickering = true;
                flickerStep = 0;
                flickerStepTime = now;
                nextFlickerTime = 10000 - ((ctx.speed - 1) * 70);
                lastFlicker = now;
            }
        } else {
            if (now - flickerStepTime >= 50) {
                flickerStepTime = now;
                flickerStep++;

                switch(flickerStep) {
                    case 1:
                        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);
                        break;
                    case 2:
                        fill_solid(ctx.leds, ctx.numLeds, ctx.color);
                        break;
                    case 3:
                        fill_solid(ctx.leds, ctx.numLeds, CRGB::Black);
                        break;
                    case 4: {
                        CRGB dimColor = ctx.color;
                        dimColor.nscale8(128);
                        fill_solid(ctx.leds, ctx.numLeds, dimColor);
                        break;
                    }
                    case 5:
                        fill_solid(ctx.leds, ctx.numLeds, ctx.color);
                        break;
                    default:
                        isFlickering = false;
                        fill_solid(ctx.leds, ctx.numLeds, ctx.color);
                        break;
                }
            }
        }
    }

    const char* getName() const override {
        return "Flicker";
    }

    void reset() override {
        lastFlicker = 0;
        nextFlickerTime = 5000;
        isFlickering = false;
        flickerStep = 0;
        flickerStepTime = 0;
    }
};
