#pragma once

#include "HexTypes.h"

/**
 * Hexagon Geometry Modul
 *
 * Lädt und verarbeitet die Hexagon-Geometrie-Konfiguration.
 * Die Konfiguration beschreibt wie die Hexagone physisch
 * angeordnet und verkabelt sind.
 */

// Konfigurationskonstanten
#define MAX_HEXAGONS 12
#define NUM_LEDS 408           // 12 Hexagons × 34 LEDs (Maximum)
#define LEDS_PER_HEXAGON 34
#define NUM_HEXAGONS 12        // Maximum, tatsächliche Anzahl aus Config

// Globale Geometrie-Daten
extern HexGeometry hexGeometry[];
extern uint8_t numConfiguredHexagons;
extern bool geometryConfigLoaded;

// LED-Koordinaten Arrays
extern float ledGlobalY[];
extern float ledGlobalX[];

// Hexagon-Segmente
extern HexSegment hexagons[];

// Lädt die Geometrie-Konfiguration basierend auf IP-Adresse
void loadGeometryConfig();

// Parst die Geometrie-Konfiguration aus einem String
bool parseGeometryConfig(const String& config);

// Berechnet globale Positionen der Hexagone
void calculateGlobalPositions();

// Berechnet LED-Koordinaten für alle LEDs
void calculateLEDCoordinates();

// Hilfsfunktionen für Geometrie-Berechnungen
uint8_t getSideForLED(uint8_t ledInHex, uint8_t startSide, bool clockwise);
uint8_t getOffsetToSide1(uint8_t startSide, bool clockwise);
uint8_t getHarmonizedLED(uint8_t hexIndex, uint8_t logicalPos, bool reverseDir = false);
uint8_t getLogicalFromPhysical(uint8_t hexIndex, uint8_t physicalPos);
float getLEDLocalY(uint8_t hexIndex, uint8_t ledInHex);
float getLEDLocalX(uint8_t hexIndex, uint8_t ledInHex);
float getLEDPreciseY(uint8_t hexIndex, uint8_t ledInHex);
