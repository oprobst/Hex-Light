#include "ConfigManager.h"
#include <Preferences.h>
#include <FastLED.h>

// Externe Variablen aus main.cpp
extern bool powerOn;
extern uint8_t globalBrightness;
extern uint8_t currentMode;
extern CRGB currentColor;
extern uint8_t animationSpeed;

// Preferences Instanz
static Preferences preferences;

void loadConfig() {
    preferences.begin("hexlight", false);  // Read-Write Mode

    // Lade gespeicherte Werte (mit Defaults falls nicht vorhanden)
    powerOn = preferences.getBool("powerOn", true);
    globalBrightness = preferences.getUChar("brightness", 128);
    currentMode = preferences.getUChar("mode", 0);
    animationSpeed = preferences.getUChar("speed", 50);

    // Farbe laden (als 3 separate Bytes)
    currentColor.r = preferences.getUChar("color_r", 255);
    currentColor.g = preferences.getUChar("color_g", 255);
    currentColor.b = preferences.getUChar("color_b", 255);

    preferences.end();

    Serial.println("Config loaded from Preferences:");
    Serial.printf("  Power: %s\n", powerOn ? "ON" : "OFF");
    Serial.printf("  Brightness: %d\n", globalBrightness);
    Serial.printf("  Mode: %d\n", currentMode);
    Serial.printf("  Speed: %d\n", animationSpeed);
    Serial.printf("  Color: R=%d G=%d B=%d\n", currentColor.r, currentColor.g, currentColor.b);

    // Brightness auf FastLED anwenden
    FastLED.setBrightness(globalBrightness);
}

void saveConfig() {
    preferences.begin("hexlight", false);  // Read-Write Mode

    preferences.putBool("powerOn", powerOn);
    preferences.putUChar("brightness", globalBrightness);
    preferences.putUChar("mode", currentMode);
    preferences.putUChar("speed", animationSpeed);
    preferences.putUChar("color_r", currentColor.r);
    preferences.putUChar("color_g", currentColor.g);
    preferences.putUChar("color_b", currentColor.b);

    preferences.end();

    Serial.println("Config saved to Preferences");
}
