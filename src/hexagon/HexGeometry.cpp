#include "HexGeometry.h"
#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "credentials.h"

// Globale Geometrie-Daten
HexGeometry hexGeometry[MAX_HEXAGONS];
uint8_t numConfiguredHexagons = 0;
bool geometryConfigLoaded = false;

// LED-Koordinaten Arrays
float ledGlobalY[NUM_LEDS];
float ledGlobalX[NUM_LEDS];

// Hexagon-Segmente (bis zu 12 Hexagone mit jeweils 34 LEDs @ 55cm)
HexSegment hexagons[NUM_HEXAGONS] = {
    {0, 34, true},      // Hexagon 1: LEDs 0-33
    {34, 34, true},     // Hexagon 2: LEDs 34-67
    {68, 34, true},     // Hexagon 3: LEDs 68-101
    {102, 34, true},    // Hexagon 4: LEDs 102-135
    {136, 34, true},    // Hexagon 5: LEDs 136-169
    {170, 34, true},    // Hexagon 6: LEDs 170-203
    {204, 34, true},    // Hexagon 7: LEDs 204-237
    {238, 34, true},    // Hexagon 8: LEDs 238-271
    {272, 34, true},    // Hexagon 9: LEDs 272-305
    {306, 34, true},    // Hexagon 10: LEDs 306-339
    {340, 34, true},    // Hexagon 11: LEDs 340-373
    {374, 34, true}     // Hexagon 12: LEDs 374-407
};

/**
 * Ermittelt die Seite (1-6) für eine LED-Position innerhalb eines Hexagons
 * Berücksichtigt Startseite und Wicklungsrichtung
 */
uint8_t getSideForLED(uint8_t ledInHex, uint8_t startSide, bool clockwise) {
    // 34 LEDs auf 6 Seiten: 6+6+6+6+5+5 = 34
    // Seiten haben ca. 5-6 LEDs
    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};  // = 34 total

    uint8_t ledCount = 0;
    uint8_t sideOffset = 0;

    for (int i = 0; i < 6; i++) {
        ledCount += ledsPerSide[i];
        if (ledInHex < ledCount) {
            sideOffset = i;
            break;
        }
    }

    // Seite berechnen basierend auf Startseite und Richtung
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
 * @param startSide Startseite (1-6)
 * @param clockwise true = Uhrzeigersinn
 * @return Anzahl LEDs von Position 0 bis Seite 1
 */
uint8_t getOffsetToSide1(uint8_t startSide, bool clockwise) {
    // LEDs pro Seite: 6+6+6+6+5+5 = 34
    // Kumulative LEDs bis zum START jeder Seite (0-basiert)
    const uint8_t sideStartLED[] = {0, 6, 12, 18, 24, 29}; // Seite 1,2,3,4,5,6

    // Wie viele Seiten müssen wir "vorwärts" gehen, um von startSide zu Seite 1 zu kommen?
    int sidesToSide1;
    if (clockwise) {
        // Im Uhrzeigersinn: Seite 2 -> braucht 5 Seiten vorwärts um zu Seite 1 zu kommen
        // Seite 1 -> 0 Seiten, Seite 2 -> 5, Seite 3 -> 4, ...
        sidesToSide1 = (7 - startSide) % 6;
    } else {
        // Gegen Uhrzeigersinn: Seite 2 -> braucht 1 Seite vorwärts
        // Seite 1 -> 0, Seite 2 -> 1, Seite 3 -> 2, ...
        sidesToSide1 = (startSide - 1);
    }

    // Berechne LEDs bis zur Seite 1
    uint8_t offset = 0;
    int currentSide = startSide - 1; // 0-basiert
    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};

    for (int i = 0; i < sidesToSide1; i++) {
        offset += ledsPerSide[currentSide];
        if (clockwise) {
            currentSide = (currentSide + 1) % 6;
        } else {
            currentSide = (currentSide + 5) % 6; // -1 mod 6
        }
    }

    return offset;
}

/**
 * Berechnet die "harmonisierte" physische LED-Position für ein Hexagon
 * Alle Hexagone werden so behandelt, als würden sie bei Seite 1 (oben) starten
 * und im Uhrzeigersinn laufen.
 *
 * @param hexIndex Index des Hexagons (0-basiert)
 * @param logicalPos Logische Position (0 = oben/Seite 1, im Uhrzeigersinn)
 * @param reverseDir Wenn true, Animation läuft gegen den Uhrzeigersinn
 * @return Physische LED-Position innerhalb des Hexagons (0-33)
 */
uint8_t getHarmonizedLED(uint8_t hexIndex, uint8_t logicalPos, bool reverseDir) {
    if (!geometryConfigLoaded || hexIndex >= numConfiguredHexagons) {
        // Ohne Konfiguration: direkte Zuordnung
        return logicalPos % LEDS_PER_HEXAGON;
    }

    HexGeometry& geo = hexGeometry[hexIndex];

    // Offset zur ersten LED von Seite 1 berechnen
    uint8_t side1Start = getOffsetToSide1(geo.startSide, geo.clockwise);

    // Für UZS-Verkabelung: Start ist erste LED von Seite 1
    // Für gegen-UZS-Verkabelung: Start ist LETZTE LED von Seite 1 (Seite 1 hat 6 LEDs)
    uint8_t logicalStartLED;
    if (geo.clockwise) {
        logicalStartLED = side1Start;
    } else {
        logicalStartLED = side1Start + 5;  // +5 weil Seite 1 hat 6 LEDs (0-5)
    }

    // Position anpassen wenn Animation rückwärts laufen soll
    int effectivePos = reverseDir ? (LEDS_PER_HEXAGON - logicalPos) % LEDS_PER_HEXAGON : logicalPos;

    // Physische Position berechnen
    int physicalPos;
    if (geo.clockwise) {
        // UZS-Verkabelung: vorwärts zählen
        physicalPos = (logicalStartLED + effectivePos) % LEDS_PER_HEXAGON;
    } else {
        // Gegen-UZS-Verkabelung: rückwärts zählen
        physicalPos = (logicalStartLED - effectivePos + LEDS_PER_HEXAGON) % LEDS_PER_HEXAGON;
    }

    return physicalPos;
}

/**
 * Berechnet die logische Position (0 = oben, im Uhrzeigersinn) aus einer physischen LED-Position
 * Inverse von getHarmonizedLED
 */
uint8_t getLogicalFromPhysical(uint8_t hexIndex, uint8_t physicalPos) {
    if (!geometryConfigLoaded || hexIndex >= numConfiguredHexagons) {
        return physicalPos % LEDS_PER_HEXAGON;
    }

    HexGeometry& geo = hexGeometry[hexIndex];

    // Offset zur ersten LED von Seite 1
    uint8_t side1Start = getOffsetToSide1(geo.startSide, geo.clockwise);

    // Start-LED (wie in getHarmonizedLED)
    uint8_t logicalStartLED;
    if (geo.clockwise) {
        logicalStartLED = side1Start;
    } else {
        logicalStartLED = side1Start + 5;
    }

    // Inverse Berechnung
    int logicalPos;
    if (geo.clockwise) {
        logicalPos = (physicalPos - logicalStartLED + LEDS_PER_HEXAGON) % LEDS_PER_HEXAGON;
    } else {
        logicalPos = (logicalStartLED - physicalPos + LEDS_PER_HEXAGON) % LEDS_PER_HEXAGON;
    }

    return logicalPos;
}

/**
 * Berechnet die lokale Y-Koordinate einer LED innerhalb eines Hexagons
 * Basierend auf der Seite, auf der sich die LED befindet
 * Hexagon mit flacher Seite oben: Seite 1 = oben (y=1.0), Seite 4 = unten (y=0.0)
 */
float getLEDLocalY(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];
    uint8_t side = getSideForLED(ledInHex, geo.startSide, geo.clockwise);

    // Y-Werte für jede Seite (flat-top Hexagon)
    //     _____ Seite 1 (y=1.0)
    //    /     \
    // 6 /       \ 2
    //   \       /
    // 5  \_____/ 3
    //     Seite 4 (y=0.0)
    const float sideY[] = {0.0f, 1.0f, 0.75f, 0.25f, 0.0f, 0.25f, 0.75f}; // Index 1-6
    return sideY[side];
}

/**
 * Berechnet die lokale X-Koordinate einer LED innerhalb eines Hexagons
 */
float getLEDLocalX(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];
    uint8_t side = getSideForLED(ledInHex, geo.startSide, geo.clockwise);

    // X-Werte für jede Seite (flat-top Hexagon)
    const float sideX[] = {0.0f, 0.5f, 1.0f, 1.0f, 0.5f, 0.0f, 0.0f}; // Index 1-6
    return sideX[side];
}

/**
 * Berechnet die präzise lokale Y-Koordinate einer LED innerhalb eines Hexagons
 * Im Gegensatz zu getLEDLocalY, die nur Seiten-basierte Y-Werte liefert,
 * interpoliert diese Funktion die Y-Position entlang der schrägen Seiten.
 *
 * Hexagon mit flacher Seite oben:
 *      _____ Seite 1 (y=1.0, konstant)
 *     /     \
 *  6 /       \ 2  (y interpoliert 0.5-1.0 / 1.0-0.5)
 *    \       /
 *  5  \_____/ 3   (y interpoliert 0.0-0.5 / 0.5-0.0)
 *      Seite 4 (y=0.0, konstant)
 */
float getLEDPreciseY(uint8_t hexIndex, uint8_t ledInHex) {
    if (hexIndex >= numConfiguredHexagons) return 0.5f;

    HexGeometry& geo = hexGeometry[hexIndex];

    // LED-Verteilung pro Seite: 6+6+6+6+5+5 = 34
    const uint8_t ledsPerSide[] = {6, 6, 6, 6, 5, 5};
    const uint8_t sideStart[] = {0, 6, 12, 18, 24, 29};  // Kumulative Startpositionen

    // Finde die Seite und Position innerhalb der Seite
    uint8_t sideIndex = 0;
    uint8_t posInSide = 0;

    for (int i = 0; i < 6; i++) {
        if (ledInHex < sideStart[i] + ledsPerSide[i]) {
            sideIndex = i;
            posInSide = ledInHex - sideStart[i];
            break;
        }
    }

    // Berechne die logische Seite basierend auf Startseite und Richtung
    int8_t logicalSide;
    if (geo.clockwise) {
        logicalSide = ((geo.startSide - 1) + sideIndex) % 6 + 1;
    } else {
        logicalSide = ((geo.startSide - 1) - sideIndex + 6) % 6 + 1;
    }

    // Berechne den Fortschritt innerhalb der Seite (0.0 - 1.0)
    float progress = (float)posInSide / (ledsPerSide[sideIndex] - 1);
    if (ledsPerSide[sideIndex] <= 1) progress = 0.5f;

    // Bei gegen-UZS-Richtung ist der Fortschritt umgekehrt
    if (!geo.clockwise) {
        progress = 1.0f - progress;
    }

    // Y-Werte basierend auf logischer Seite und Position
    float y;
    switch (logicalSide) {
        case 1:  // Oben (horizontal, konstant y=1.0)
            y = 1.0f;
            break;
        case 2:  // Rechts oben (y geht von 1.0 nach 0.5)
            y = 1.0f - (progress * 0.5f);
            break;
        case 3:  // Rechts unten (y geht von 0.5 nach 0.0)
            y = 0.5f - (progress * 0.5f);
            break;
        case 4:  // Unten (horizontal, konstant y=0.0)
            y = 0.0f;
            break;
        case 5:  // Links unten (y geht von 0.0 nach 0.5)
            y = 0.0f + (progress * 0.5f);
            break;
        case 6:  // Links oben (y geht von 0.5 nach 1.0)
            y = 0.5f + (progress * 0.5f);
            break;
        default:
            y = 0.5f;
    }

    return y;
}

/**
 * Parst die Geometrie-Konfiguration aus einem String
 * Format: "1 2U 3H2; 2 1U 3H3; ..."
 * - Hexagon-Nummer
 * - Startseite + Richtung (U=Uhrzeigersinn, G=Gegen)
 * - Optional: Nachbarn (SeiteHNachbar-Nummer)
 */
bool parseGeometryConfig(const String& config) {
    Serial.println("Parsing geometry config: " + config);

    // Initialisiere alle Hexagone
    for (int i = 0; i < MAX_HEXAGONS; i++) {
        hexGeometry[i].hexNumber = 0;
        hexGeometry[i].startSide = 1;
        hexGeometry[i].clockwise = true;
        hexGeometry[i].globalY = 0.0f;
        hexGeometry[i].globalX = 0.0f;
        for (int j = 0; j < 6; j++) {
            hexGeometry[i].neighbors[j].side = 0;
            hexGeometry[i].neighbors[j].neighborHex = 0;
        }
    }

    numConfiguredHexagons = 0;
    String remaining = config;
    remaining.trim();

    while (remaining.length() > 0 && numConfiguredHexagons < MAX_HEXAGONS) {
        // Finde das nächste Semikolon oder Ende
        int semiPos = remaining.indexOf(';');
        String hexDef;
        if (semiPos >= 0) {
            hexDef = remaining.substring(0, semiPos);
            remaining = remaining.substring(semiPos + 1);
            remaining.trim();
        } else {
            hexDef = remaining;
            remaining = "";
        }

        hexDef.trim();
        if (hexDef.length() == 0) continue;

        // Parse Hexagon-Definition: "1 2U 3H2"
        HexGeometry& geo = hexGeometry[numConfiguredHexagons];

        // Teile die Definition in Tokens
        int spacePos1 = hexDef.indexOf(' ');
        if (spacePos1 < 0) continue;

        // Hexagon-Nummer
        geo.hexNumber = hexDef.substring(0, spacePos1).toInt();

        String rest = hexDef.substring(spacePos1 + 1);
        rest.trim();

        // Startseite und Richtung (z.B. "2U")
        int spacePos2 = rest.indexOf(' ');
        String sideDir;
        String neighbors;

        if (spacePos2 >= 0) {
            sideDir = rest.substring(0, spacePos2);
            neighbors = rest.substring(spacePos2 + 1);
        } else {
            sideDir = rest;
            neighbors = "";
        }

        // Parse Startseite und Richtung
        int dirPos = sideDir.indexOf('U');
        if (dirPos >= 0) {
            geo.startSide = sideDir.substring(0, dirPos).toInt();
            geo.clockwise = true;
        } else {
            dirPos = sideDir.indexOf('G');
            if (dirPos >= 0) {
                geo.startSide = sideDir.substring(0, dirPos).toInt();
                geo.clockwise = false;
            }
        }

        // Parse Nachbarn (z.B. "3H2 5H4")
        neighbors.trim();
        while (neighbors.length() > 0) {
            int nextSpace = neighbors.indexOf(' ');
            String neighborDef;
            if (nextSpace >= 0) {
                neighborDef = neighbors.substring(0, nextSpace);
                neighbors = neighbors.substring(nextSpace + 1);
                neighbors.trim();
            } else {
                neighborDef = neighbors;
                neighbors = "";
            }

            // Parse "3H2" -> Seite 3, Nachbar Hexagon 2
            int hPos = neighborDef.indexOf('H');
            if (hPos > 0) {
                uint8_t side = neighborDef.substring(0, hPos).toInt();
                uint8_t neighborHex = neighborDef.substring(hPos + 1).toInt();

                if (side >= 1 && side <= 6) {
                    geo.neighbors[side - 1].side = side;
                    geo.neighbors[side - 1].neighborHex = neighborHex;
                }
            }
        }

        Serial.printf("  Hex %d: Start=%d, CW=%d\n",
                      geo.hexNumber, geo.startSide, geo.clockwise);

        numConfiguredHexagons++;
    }

    Serial.printf("Parsed %d hexagons\n", numConfiguredHexagons);
    return numConfiguredHexagons > 0;
}

/**
 * Berechnet die globalen Positionen aller Hexagone basierend auf Nachbarschaften
 * Verwendet BFS von Hexagon 1 ausgehend
 */
void calculateGlobalPositions() {
    if (numConfiguredHexagons == 0) return;

    // Hexagon 1 als Referenzpunkt
    hexGeometry[0].globalX = 0.0f;
    hexGeometry[0].globalY = 0.0f;

    // Array um zu tracken, welche Hexagone bereits positioniert sind
    bool positioned[MAX_HEXAGONS] = {false};
    positioned[0] = true;

    // Y/X-Offsets basierend auf Nachbarseite (flat-top Hexagon)
    // Wenn Nachbar an Seite X liegt, ist dessen Position:
    const float sideOffsetY[] = {0.0f, 1.0f, 0.5f, -0.5f, -1.0f, -0.5f, 0.5f}; // Index 1-6
    const float sideOffsetX[] = {0.0f, 0.0f, 0.866f, 0.866f, 0.0f, -0.866f, -0.866f}; // Index 1-6

    // Einfache Iteration (mehrfach durchlaufen für Kaskaden)
    for (int iteration = 0; iteration < numConfiguredHexagons; iteration++) {
        for (int i = 0; i < numConfiguredHexagons; i++) {
            if (!positioned[i]) continue;

            HexGeometry& geo = hexGeometry[i];

            // Prüfe alle Nachbarn
            for (int side = 0; side < 6; side++) {
                uint8_t neighborHex = geo.neighbors[side].neighborHex;
                if (neighborHex == 0) continue;

                // Finde den Index des Nachbarn
                for (int j = 0; j < numConfiguredHexagons; j++) {
                    if (hexGeometry[j].hexNumber == neighborHex && !positioned[j]) {
                        // Berechne Position des Nachbarn
                        hexGeometry[j].globalX = geo.globalX + sideOffsetX[side + 1];
                        hexGeometry[j].globalY = geo.globalY + sideOffsetY[side + 1];
                        positioned[j] = true;

                        Serial.printf("  Hex %d positioned at (%.2f, %.2f)\n",
                                      neighborHex, hexGeometry[j].globalX, hexGeometry[j].globalY);
                    }
                }
            }
        }
    }
}

/**
 * Berechnet die globalen Y-Koordinaten für alle LEDs
 */
void calculateLEDCoordinates() {
    // Finde Min/Max Y für Normalisierung
    float minY = 999.0f, maxY = -999.0f;

    for (int i = 0; i < numConfiguredHexagons; i++) {
        float hexY = hexGeometry[i].globalY;
        // Hexagon-Höhe addieren (ca. 1.0 Einheiten)
        if (hexY - 0.5f < minY) minY = hexY - 0.5f;
        if (hexY + 0.5f > maxY) maxY = hexY + 0.5f;
    }

    float range = maxY - minY;
    if (range < 0.01f) range = 1.0f;

    Serial.printf("Y Range: %.2f to %.2f\n", minY, maxY);

    // Berechne Y-Koordinate für jede LED
    for (int i = 0; i < numConfiguredHexagons; i++) {
        if (i >= NUM_HEXAGONS) break;

        HexGeometry& geo = hexGeometry[i];
        uint16_t hexStart = hexagons[i].startLED;
        uint16_t hexCount = hexagons[i].count;

        for (int j = 0; j < hexCount; j++) {
            uint16_t ledIndex = hexStart + j;
            if (ledIndex >= NUM_LEDS) break;

            // Lokale Y-Position im Hexagon
            float localY = getLEDLocalY(i, j);

            // Globale Y-Position (Hexagon-Position + lokale Offset)
            float globalY = geo.globalY + (localY - 0.5f);

            // Normalisieren auf 0.0 - 1.0
            ledGlobalY[ledIndex] = (globalY - minY) / range;

            // X-Koordinate ähnlich
            float localX = getLEDLocalX(i, j);
            ledGlobalX[ledIndex] = geo.globalX + (localX - 0.5f);
        }
    }

    Serial.println("LED coordinates calculated");
}

/**
 * Lädt die Geometrie-Konfiguration basierend auf der IP-Adresse
 * Sucht nach /config/<IP>.cfg im LittleFS
 */
void loadGeometryConfig() {
    String ipAddress;
    if (USE_AP_MODE) {
        ipAddress = WiFi.softAPIP().toString();
    } else {
        ipAddress = WiFi.localIP().toString();
    }

    String configPath = "/config/" + ipAddress + ".cfg";
    Serial.println("Looking for geometry config: " + configPath);

    if (LittleFS.exists(configPath)) {
        File configFile = LittleFS.open(configPath, "r");
        if (configFile) {
            String config = configFile.readString();
            configFile.close();

            if (parseGeometryConfig(config)) {
                calculateGlobalPositions();
                calculateLEDCoordinates();
                geometryConfigLoaded = true;
                Serial.println("Geometry config loaded successfully!");

                // Debug: Zeige berechnete Werte für jedes Hexagon
                Serial.println("=== Hexagon Harmonization Debug ===");
                for (int h = 0; h < numConfiguredHexagons; h++) {
                    HexGeometry& geo = hexGeometry[h];
                    uint8_t offset = getOffsetToSide1(geo.startSide, geo.clockwise);
                    uint8_t startLED = geo.clockwise ? offset : offset + 5;
                    Serial.printf("Hex %d: start=%d, cw=%d, side1Offset=%d, logicalStart=%d\n",
                                  geo.hexNumber, geo.startSide, geo.clockwise, offset, startLED);
                    Serial.printf("  logPos: 0->%d, 1->%d, 2->%d, 6->%d, 33->%d\n",
                                  getHarmonizedLED(h, 0, false),
                                  getHarmonizedLED(h, 1, false),
                                  getHarmonizedLED(h, 2, false),
                                  getHarmonizedLED(h, 6, false),
                                  getHarmonizedLED(h, 33, false));
                }
                Serial.println("===================================");
            }
        }
    } else {
        Serial.println("No geometry config found for this IP - using default");

        // Default-Konfiguration: Alle Hexagone nebeneinander (keine Geometrie-Effekte)
        geometryConfigLoaded = false;

        // Setze Standard-Y-Koordinaten (linear entlang des Strips)
        for (int i = 0; i < NUM_LEDS; i++) {
            ledGlobalY[i] = (float)i / NUM_LEDS;
            ledGlobalX[i] = 0.0f;
        }
    }
}
