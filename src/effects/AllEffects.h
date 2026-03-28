#pragma once

// Effect system includes
#include "Effect.h"
#include "EffectContext.h"
#include "EffectRegistry.h"

// Basic effects
#include "basic/SolidEffect.h"
#include "basic/RainbowEffect.h"
#include "basic/PulseEffect.h"
#include "basic/RainbowHexEffect.h"
#include "basic/SequentialBlinkEffect.h"
#include "basic/BreatheEffect.h"
#include "basic/StrobeEffect.h"
#include "basic/FlickerEffect.h"
#include "basic/StrobeLongEffect.h"
#include "basic/HeartbeatEffect.h"
#include "basic/SparkleEffect.h"
#include "basic/RainbowSparkleEffect.h"

// Circle effects
#include "circle/RunningLightEffect.h"
#include "circle/ChaseCometEffect.h"
#include "circle/SyncCircleEffect.h"
#include "circle/ComplementaryCircleEffect.h"
#include "circle/AlternatingCircleEffect.h"
#include "circle/AlternatingComplementaryEffect.h"

// Geometry effects
#include "geometry/VerticalRainbowEffect.h"
#include "geometry/HorizontalRainbowEffect.h"
#include "geometry/VerticalGradientEffect.h"
#include "geometry/VerticalWaveEffect.h"
#include "geometry/VerticalWaveLEDEffect.h"
#include "geometry/RadialWaveOutEffect.h"
#include "geometry/RadialWaveInEffect.h"
#include "geometry/VortexEffect.h"
#include "geometry/FireEffect.h"
#include "geometry/AuroraEffect.h"
#include "geometry/PlasmaEffect.h"
#include "geometry/LavaLampEffect.h"
#include "geometry/WaveCollisionEffect.h"
#include "geometry/ColorWheelEffect.h"
#include "geometry/BreathingCenterEffect.h"
#include "geometry/WaveWallEffect.h"

// Special effects
#include "special/RainEffect.h"
#include "special/BouncingBallEffect.h"
#include "special/MatrixRainEffect.h"
#include "special/CheckerboardEffect.h"
#include "special/MeteorEffect.h"

// Debug effects
#include "debug/PhysicalCircleEffect.h"
#include "debug/HarmonizedCircleEffect.h"

// Effect instances (static allocation)
namespace Effects {
    // Basic (0-10)
    static SolidEffect solid;
    static RainbowEffect rainbow;
    static PulseEffect pulse;
    static RainbowHexEffect rainbowHex;
    static SequentialBlinkEffect sequentialBlink;
    static BreatheEffect breathe;
    static StrobeEffect strobe;
    static FlickerEffect flicker;
    static StrobeLongEffect strobeLong;
    static HeartbeatEffect heartbeat;
    static SparkleEffect sparkle;
    static RainbowSparkleEffect rainbowSparkle;

    // Circle (8-12, 20-21)
    static RunningLightEffect runningLight;
    static ChaseCometEffect chaseComet;
    static SyncCircleEffect syncCircle;
    static ComplementaryCircleEffect complementaryCircle;
    static AlternatingCircleEffect alternatingCircle;
    static AlternatingComplementaryEffect alternatingComplementary;

    // Geometry (13-19, 24-27, 32, 34-36, 39)
    static VerticalRainbowEffect verticalRainbow;
    static HorizontalRainbowEffect horizontalRainbow;
    static VerticalGradientEffect verticalGradient;
    static VerticalWaveEffect verticalWave;
    static VerticalWaveLEDEffect verticalWaveLED;
    static RadialWaveOutEffect radialWaveOut;
    static RadialWaveInEffect radialWaveIn;
    static VortexEffect vortex;
    static FireEffect fire;
    static AuroraEffect aurora;
    static PlasmaEffect plasma;
    static LavaLampEffect lavaLamp;
    static WaveCollisionEffect waveCollision;
    static ColorWheelEffect colorWheel;
    static BreathingCenterEffect breathingCenter;
    static WaveWallEffect waveWall;

    // Special (30-31, 33, 37-38)
    static RainEffect rain;
    static BouncingBallEffect bouncingBall;
    static MatrixRainEffect matrixRain;
    static CheckerboardEffect checkerboard;
    static MeteorEffect meteor;

    // Debug (22-23)
    static PhysicalCircleEffect physicalCircle;
    static HarmonizedCircleEffect harmonizedCircle;

    /**
     * Register all effects with the EffectRegistry
     * Call this once during setup()
     */
    inline void registerAll() {
        EffectRegistry& reg = EffectRegistry::getInstance();

        // Basic effects (0-7, 10)
        reg.registerEffect(0, &solid);
        reg.registerEffect(1, &rainbow);
        reg.registerEffect(2, &pulse);
        reg.registerEffect(3, &rainbowHex);
        reg.registerEffect(4, &sequentialBlink);
        reg.registerEffect(5, &breathe);
        reg.registerEffect(6, &strobe);
        reg.registerEffect(7, &flicker);
        reg.registerEffect(10, &strobeLong);
        reg.registerEffect(28, &heartbeat);
        reg.registerEffect(29, &sparkle);
        reg.registerEffect(40, &rainbowSparkle);

        // Circle effects (8-9, 11-12, 20-21)
        reg.registerEffect(8, &runningLight);
        reg.registerEffect(9, &chaseComet);
        reg.registerEffect(11, &syncCircle);
        reg.registerEffect(12, &complementaryCircle);
        reg.registerEffect(20, &alternatingCircle);
        reg.registerEffect(21, &alternatingComplementary);

        // Geometry effects (13-19, 24-27, 32, 34-36, 39)
        reg.registerEffect(13, &verticalRainbow);
        reg.registerEffect(14, &horizontalRainbow);
        reg.registerEffect(15, &verticalGradient);
        reg.registerEffect(16, &verticalWave);
        reg.registerEffect(17, &fire);
        reg.registerEffect(18, &aurora);
        reg.registerEffect(19, &plasma);
        reg.registerEffect(24, &verticalWaveLED);
        reg.registerEffect(25, &radialWaveOut);
        reg.registerEffect(26, &radialWaveIn);
        reg.registerEffect(27, &vortex);
        reg.registerEffect(32, &lavaLamp);
        reg.registerEffect(34, &waveCollision);
        reg.registerEffect(35, &colorWheel);
        reg.registerEffect(36, &breathingCenter);
        reg.registerEffect(39, &waveWall);

        // Special effects (30-31, 33, 37-38)
        reg.registerEffect(30, &rain);
        reg.registerEffect(31, &bouncingBall);
        reg.registerEffect(33, &matrixRain);
        reg.registerEffect(37, &checkerboard);
        reg.registerEffect(38, &meteor);

        // Debug effects (22-23)
        reg.registerEffect(22, &physicalCircle);
        reg.registerEffect(23, &harmonizedCircle);
    }
}
