#pragma once

#include "../Effect.h"
#include <cmath>

class BouncingBallEffect : public Effect {
private:
    float ballY = 1.0f;
    float ballVelocity = 0.0f;
    unsigned long lastUpdate = 0;

    static constexpr float gravity = 0.003f;
    static constexpr float bounceFactor = 0.85f;
    static constexpr float ballSize = 0.15f;

public:
    void update(EffectContext& ctx) override {
        unsigned long now = millis();
        unsigned long updateInterval = 25 - (ctx.speed / 5);

        if (now - lastUpdate >= updateInterval) {
            lastUpdate = now;

            ballVelocity -= gravity;
            ballY += ballVelocity;

            if (ballY <= 0.0f) {
                ballY = 0.0f;
                ballVelocity = -ballVelocity * bounceFactor;

                if (fabs(ballVelocity) < 0.01f) {
                    ballY = 1.0f;
                    ballVelocity = 0.0f;
                }
            }
        }

        for (int h = 0; h < ctx.numHexagons; h++) {
            if (!ctx.hexagons[h].enabled) continue;

            for (int j = 0; j < ctx.hexagons[h].count; j++) {
                uint16_t ledIndex = ctx.hexagons[h].startLED + j;
                float ledY = ctx.getLEDPreciseY(h, j);

                float dist = fabs(ledY - ballY);
                if (dist < ballSize) {
                    uint8_t brightness = (uint8_t)((1.0f - dist / ballSize) * 255);
                    CRGB color = ctx.color;
                    color.nscale8(brightness);
                    ctx.leds[ledIndex] = color;
                } else {
                    ctx.leds[ledIndex] = CRGB::Black;
                }
            }
        }
    }

    const char* getName() const override {
        return "Bouncing Ball";
    }

    void reset() override {
        ballY = 1.0f;
        ballVelocity = 0.0f;
        lastUpdate = 0;
    }
};
