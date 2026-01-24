#pragma once

#include <Arduino.h>

// Hexagon Segment Definition
struct HexSegment {
    uint16_t startLED;
    uint16_t count;
    bool enabled;
};

// Nachbar-Verbindung
struct HexNeighbor {
    uint8_t side;        // An welcher Seite (1-6)
    uint8_t neighborHex; // Welches Hexagon (0 = kein Nachbar)
};

// Hexagon-Geometrie-Konfiguration
struct HexGeometry {
    uint8_t hexNumber;       // 1-basiert (wie in Config)
    uint8_t startSide;       // Startseite der LEDs (1-6)
    bool clockwise;          // true = Uhrzeigersinn (U), false = gegen (G)
    HexNeighbor neighbors[6]; // Nachbarn pro Seite
    float globalY;           // Berechnete globale Y-Position
    float globalX;           // Berechnete globale X-Position
};
