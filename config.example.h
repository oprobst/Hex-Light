/**
 * Hex-Light Konfigurationsvorlage
 *
 * Diese Datei kopieren nach src/config.h und anpassen!
 * src/config.h wird durch .gitignore ausgeschlossen.
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== WiFi Konfiguration =====
#define USE_AP_MODE true              // true = Access Point, false = Station Mode

// Access Point Modus Einstellungen
#define WIFI_AP_SSID "Hex-Light-AP"
#define WIFI_AP_PASSWORD "hexlight123"

// Station Modus Einstellungen (für Heimnetzwerk)
#define WIFI_STA_SSID "MeinWLAN"
#define WIFI_STA_PASSWORD "meinpasswort"

// ===== LED Konfiguration =====
#define LED_PIN 5                     // GPIO Pin für LED Daten
#define NUM_LEDS 408                  // Gesamtanzahl der LEDs (12 Hexagone × 34 LEDs)
#define LEDS_PER_HEXAGON 34           // 55cm Streifen @ 60 LEDs/m = 34 LEDs (real gemessen)
#define LED_TYPE WS2811               // WS2815 ist kompatibel mit WS2811
#define COLOR_ORDER GRB               // Farbordnung (GRB für WS2815)

// Helligkeit
#define DEFAULT_BRIGHTNESS 128        // Starthelligkeit (0-255)
#define MAX_BRIGHTNESS 255            // Maximale Helligkeit

// ===== Hexagon Konfiguration =====
#define NUM_HEXAGONS 12               // Anzahl der Hexagone (max. 12)

// Hexagon Segmente definieren
// Format: {startLED, anzahlLEDs, aktiviert}
// Jedes Hexagon: 55cm LED-Streifen mit 33 LEDs
struct HexSegment {
    uint16_t startLED;
    uint16_t count;
    bool enabled;
};

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

// ===== Webserver Konfiguration =====
#define WEBSERVER_PORT 80

// ===== Erweiterte Einstellungen =====
#define SERIAL_BAUD 115200            // Serial Monitor Baudrate
#define LED_UPDATE_FPS 50             // Frames per Second für LED Updates

// Debug Modus
#define DEBUG_MODE false              // true = Ausführliche Serial Ausgabe

#endif // CONFIG_H
