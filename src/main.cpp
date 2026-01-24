#include <Arduino.h>
#include <WiFi.h>
#include <FastLED.h>
#include <LittleFS.h>
#include "effects/AllEffects.h"
#include "web/WebServer.h"
#include "mqtt/MQTTHandler.h"
#include "config/ConfigManager.h"
#include "hexagon/HexGeometry.h"
#include "network/NetworkManager.h"

// ===== LED Configuration =====
#define LED_PIN 5
#define LED_TYPE WS2811
#define COLOR_ORDER GRB

// ===== Globale Variablen =====
CRGB leds[NUM_LEDS];

// LED Steuerung
uint8_t globalBrightness = 128;
bool powerOn = true;
uint8_t currentMode = 0;
CRGB currentColor = CRGB::White;
uint8_t animationSpeed = 50;

// Effect Context
EffectContext effectCtx;

// ===== Funktionsdeklarationen =====
void setupLEDs();
void updateLEDs();
void initEffectContext();

// ===== Setup =====
void setup() {
    Serial.begin(115200);
    Serial.println("\n\n=== Hex-Light ESP32 Starting ===");

    // LED Setup
    setupLEDs();

    // Config laden
    loadConfig();

    // Filesystem
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS Mount Failed");
    }

    // WiFi Setup
    setupWiFi();

    // Hexagon-Geometrie laden (nach WiFi, da IP-basiert)
    loadGeometryConfig();

    // Effekt-System initialisieren
    Effects::registerAll();
    initEffectContext();

    // OTA Setup
    setupOTA();

    // MQTT Setup
    setupMQTT();

    // Webserver Setup
    setupWebServer();

    Serial.println("=== Setup Complete ===");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// ===== Main Loop =====
void loop() {
    handleOTA();

    // MQTT Verbindung aufrechterhalten
    if (!mqttClient.connected()) {
        mqttReconnect();
    }
    mqttClient.loop();

    updateLEDs();
    delay(20);
}

// ===== LED Setup =====
void setupLEDs() {
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(globalBrightness);

    // Initialisierung: Alle LEDs weiß
    fill_solid(leds, NUM_LEDS, CRGB::White);
    FastLED.show();

    Serial.println("LEDs initialized");
}

// ===== Effect Context Initialisierung =====
void initEffectContext() {
    effectCtx.leds = leds;
    effectCtx.numLeds = NUM_LEDS;
    effectCtx.color = currentColor;
    effectCtx.speed = animationSpeed;
    effectCtx.hexagons = hexagons;
    effectCtx.numHexagons = NUM_HEXAGONS;
    effectCtx.ledsPerHexagon = LEDS_PER_HEXAGON;
    effectCtx.ledGlobalX = ledGlobalX;
    effectCtx.ledGlobalY = ledGlobalY;
    effectCtx.hexGeometry = hexGeometry;
    effectCtx.numConfiguredHexagons = numConfiguredHexagons;
    effectCtx.geometryLoaded = geometryConfigLoaded;
    effectCtx.getLEDPreciseY = getLEDPreciseY;
    effectCtx.getHarmonizedLED = getHarmonizedLED;

    // Zentrum berechnen
    effectCtx.centerX = 0.0f;
    effectCtx.centerY = 0.0f;
    if (geometryConfigLoaded) {
        for (int h = 0; h < numConfiguredHexagons; h++) {
            effectCtx.centerX += hexGeometry[h].globalX;
            effectCtx.centerY += hexGeometry[h].globalY;
        }
        effectCtx.centerX /= numConfiguredHexagons;
        effectCtx.centerY /= numConfiguredHexagons;
    }

    Serial.println("Effect context initialized");
}

// ===== LED Update Loop =====
void updateLEDs() {
    if (!powerOn) {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        return;
    }

    // Update context with current settings
    effectCtx.color = currentColor;
    effectCtx.speed = animationSpeed;

    // Run the current effect
    EffectRegistry::getInstance().runEffect(currentMode, effectCtx);

    FastLED.show();
}

