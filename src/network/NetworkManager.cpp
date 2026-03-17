#include "NetworkManager.h"
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <FastLED.h>
#include "credentials.h"
#include "../hexagon/HexGeometry.h"

// Externe Variablen aus main.cpp
extern CRGB leds[];

static constexpr unsigned long AP_TIMEOUT_MS = 180000;  // 3 Minuten
static unsigned long fallbackApStart = 0;
static bool fallbackApActive = false;

void setupWiFi() {
    if (USE_AP_MODE) {
        // Access Point Modus (dauerhaft)
        WiFi.mode(WIFI_AP);
        WiFi.softAP(AP_SSID, WIFI_PASSWORD);
        Serial.println("WiFi AP Mode started");
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        // Station Modus
        WiFi.mode(WIFI_STA);
        WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

        Serial.print("Connecting to WiFi");
        int attempts = 0;
        while (WiFi.status() != WL_CONNECTED && attempts < 20) {
            delay(500);
            Serial.print(".");
            attempts++;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nWiFi connected");
            Serial.print("IP: ");
            Serial.println(WiFi.localIP());
        } else {
            Serial.println("\nWiFi connection failed - switching to AP mode (3 min timeout)");
            WiFi.mode(WIFI_AP);
            WiFi.softAP(AP_SSID, WIFI_PASSWORD);
            fallbackApStart = millis();
            fallbackApActive = true;
        }
    }
}

void handleWiFi() {
    if (fallbackApActive && (millis() - fallbackApStart >= AP_TIMEOUT_MS)) {
        Serial.println("[WiFi] Fallback-AP Timeout - AP wird deaktiviert");
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_OFF);
        fallbackApActive = false;
    }
}

void setupOTA() {
    // Hostname setzen
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    // Passwort setzen
    ArduinoOTA.setPassword(OTA_PASSWORD);

    // Port (Standard: 3232)
    ArduinoOTA.setPort(3232);

    // Callbacks für Status-Meldungen
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("OTA Update Start: " + type);

        // LEDs ausschalten während Update
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nOTA Update Complete!");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));

        // Fortschritt auf LEDs anzeigen
        int ledProgress = map(progress, 0, total, 0, NUM_LEDS);
        fill_solid(leds, ledProgress, CRGB::Blue);
        FastLED.show();
    });

    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");

        // LEDs rot bei Fehler
        fill_solid(leds, NUM_LEDS, CRGB::Red);
        FastLED.show();
        delay(1000);
    });

    ArduinoOTA.begin();
    Serial.println("OTA Update ready!");
    Serial.print("Hostname: ");
    Serial.println(OTA_HOSTNAME);
}

void handleOTA() {
    ArduinoOTA.handle();
}
