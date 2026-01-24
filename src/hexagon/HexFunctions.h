#pragma once

#include <Arduino.h>
#include "HexTypes.h"

// Externe Referenzen (werden in main.cpp definiert)
extern HexGeometry hexGeometry[];
extern uint8_t numConfiguredHexagons;
extern bool geometryConfigLoaded;

#define LEDS_PER_HEXAGON 34
#define LEDS_PER_SIDE 6

/**
 * Ermittelt die Seite (1-6) für eine LED-Position innerhalb eines Hexagons
 */
inline uint8_t getSideForLED(uint8_t ledInHex, uint8_t startSide, bool clockwise) {
    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};
    uint8_t ledCount = 0;
    uint8_t sideOffset = 0;

    for (int i = 0; i < 6; i++) {
        ledCount += ledsPerSide[i];
        if (ledInHex < ledCount) {
            sideOffset = i;
            break;
        }
    }

    int8_t actualSide;
    if (clockwise) {
        actualSide = ((startSide - 1) + sideOffset) % 6 + 1;
    } else {
        actualSide = ((startSide - 1) - sideOffset + 6) % 6 + 1;
    }

    return actualSide;
}

/**
 * Berechnet den LED-Offset zur Seite 1 (oben) für ein Hexagon
 */
inline uint8_t getOffsetToSide1(uint8_t startSide, bool clockwise) {
    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};

    int sidesToSide1;
    if (clockwise) {
        sidesToSide1 = (7 - startSide) % 6;
    } else {
        sidesToSide1 = (startSide - 1);
    }

    uint8_t offset = 0;
    int currentSide = startSide - 1;

    for (int i = 0; i < sidesToSide1; i++) {
        offset += ledsPerSide[currentSide];
        if (clockwise) {
            currentSide = (currentSide + 1) % 6;
        } else {
            currentSide = (currentSide + 5) % 6;
        }
    }

    return offset;
}

/**
 * Berechnet die "harmonisierte" physische LED-Position für ein Hexagon
 */
inline uint8_t getHarmonizedLED(uint8_t hexIndex, uint8_t logicalPos, bool reverseDir = false) {
    if (!geometryConfigLoaded || hexIndex >= numConfiguredHexagons) {
        return logicalPos % LEDS_PER_HEXAGON;
    }

    HexGeometry& geo = hexGeometry[hexIndex];
    uint8_t side1Start = getOffsetToSide1(geo.startSide, geo.clockwise);

    uint8_t logicalStartLED;
    if (geo.clockwise) {
        logicalStartLED = side1Start;
    } else {
        logicalStartLED = side1Start + 5;
    }

    int effectivePos = reverseDir ? (LEDS_PER_HEXAGON - logicalPos) % LEDS_PER_HEXAGON : logicalPos;

    int physicalPos;
    if (geo.clockwise) {
        physicalPos = (logicalStartLED + effectivePos) % LEDS_PER_HEXAGON;
    } else {
        physicalPos = (logicalStartLED - effectivePos + LEDS_PER_HEXAGON) % LEDS_PER_HEXAGON;
    }

    return physicalPos;
}

/**
 * Berechnet die lokale Y-Koordinate einer LED (Seiten-basiert)
 */
inline float getLEDLocalY(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];
    uint8_t side = getSideForLED(ledInHex, geo.startSide, geo.clockwise);

    const float sideY[] = {0.0f, 1.0f, 0.75f, 0.25f, 0.0f, 0.25f, 0.75f};
    return sideY[side];
}

/**
 * Berechnet die lokale X-Koordinate einer LED
 */
inline float getLEDLocalX(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];
    uint8_t side = getSideForLED(ledInHex, geo.startSide, geo.clockwise);

    const float sideX[] = {0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f};
    return sideX[side];
}

/**
 * Berechnet die präzise lokale Y-Koordinate einer LED
 * Interpoliert entlang der schrägen Seiten
 */
inline float getLEDPreciseY(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];

    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};
    const uint8_t sideStart[] = {0, 6, 12, 18, 24, 29};

    uint8_t sideIndex = 0;
    uint8_t posInSide = 0;

    for (int i = 0; i < 6; i++) {
        if (ledInHex < sideStart[i] + ledsPerSide[i]) {
            sideIndex = i;
            posInSide = ledInHex - sideStart[i];
            break;
        }
    }

    int8_t logicalSide;
    if (geo.clockwise) {
        logicalSide = ((geo.startSide - 1) + sideIndex) % 6 + 1;
    } else {
        logicalSide = ((geo.startSide - 1) - sideIndex + 6) % 6 + 1;
    }

    float progress = (float)posInSide / (ledsPerSide[sideIndex] - 1);
    if (ledsPerSide[sideIndex] <= 1) progress = 0.5f;

    if (!geo.clockwise) {
        progress = 1.0f - progress;
    }

    float y;
    switch (logicalSide) {
        case 1: y = 1.0f; break;
        case 2: y = 1.0f - (progress * 0.5f); break;
        case 3: y = 0.5f - (progress * 0.5f); break;
        case 4: y = 0.0f; break;
        case 5: y = 0.0f + (progress * 0.5f); break;
        case 6: y = 0.5f + (progress * 0.5f); break;
        default: y = 0.5f;
    }

    return y;
}
