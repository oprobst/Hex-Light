#pragma once

#include "../Effect.h"

class HeartbeatEffect : public Effect {
private:
    unsigned long cycleStart = 0;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long beatDuration = 2000 - (ctx.speed * 15);

        unsigned long elapsed = now - cycleStart;
        float progress = (float)elapsed / beatDuration;

        if (progress >= 1.0f) {
            cycleStart = now;
            progress = 0.0f;
        }

        float brightness = 0.0f;

        if (progress < 0.15f) {
            brightness = progress / 0.15f;
        } else if (progress < 0.25f) {
            brightness = 1.0f - ((progress - 0.15f) / 0.10f) * 0.6f;
        } else if (progress < 0.35f) {
            brightness = 0.4f + ((progress - 0.25f) / 0.10f) * 0.6f;
        } else if (progress < 0.50f) {
            brightness = 1.0f - ((progress - 0.35f) / 0.15f);
        } else {
            brightness = 0.0f;
        }

        brightness = constrain(brightness, 0.0f, 1.0f);

        CRGB color = ctx.color;
        color.nscale8((uint8_t)(brightness * 255));
        fill_solid(ctx.leds, ctx.numLeds, color);
    }

    const char* getName() const override {
        return "Herzschlag";
    }

    void reset() override {
        cycleStart = 0;
    }
};
