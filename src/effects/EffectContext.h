#pragma once

#include <FastLED.h>
#include "../hexagon/HexTypes.h"

/**
 * EffectContext - Alle Daten die Effekte für ihre Berechnung brauchen
 * Wird bei jedem update() Aufruf übergeben
 */
struct EffectContext {
    // LED Array
    CRGB* leds;
    uint16_t numLeds;

    // Aktuelle Einstellungen
    CRGB color;
    uint8_t speed;          // 1-100

    // Hexagon-Konfiguration
    HexSegment* hexagons;
    uint8_t numHexagons;
    uint8_t ledsPerHexagon;

    // Geometrie-Daten
    float* ledGlobalX;
    float* ledGlobalY;
    HexGeometry* hexGeometry;
    uint8_t numConfiguredHexagons;
    bool geometryLoaded;

    // Zentrum (vorberechnet)
    float centerX;
    float centerY;

    // Hilfsfunktionen (als Function-Pointer für Entkopplung)
    float (*getLEDPreciseY)(uint8_t hexIndex, uint8_t ledInHex);
    uint8_t (*getHarmonizedLED)(uint8_t hexIndex, uint8_t logicalPos, bool reverseDir);
};
