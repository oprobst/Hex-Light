#include "WebServer.h"
#include "WebUI.h"
#include <ArduinoJson.h>
#include <FastLED.h>
#include <LittleFS.h>

// Build-Info Makros (aus platformio.ini)
#ifndef BUILD_TIMESTAMP
#define BUILD_TIMESTAMP 0
#endif

// Externe Variablen aus main.cpp
extern bool powerOn;
extern uint8_t globalBrightness;
extern uint8_t currentMode;
extern CRGB currentColor;
extern uint8_t animationSpeed;
extern bool geometryConfigLoaded;
extern uint8_t numConfiguredHexagons;
extern void saveConfig();

// Server-Instanz
AsyncWebServer webServer(80);

void setupWebServer() {
    // API Endpoints (müssen VOR serveStatic registriert werden!)
    webServer.on("/api/status", HTTP_GET, handleGetStatus);
    webServer.on("/api/power", HTTP_POST, handlePower);

    // POST Endpoints mit JSON Body
    webServer.on("/api/color", HTTP_POST,
        [](AsyncWebServerRequest *request){},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<128> doc;
            deserializeJson(doc, data, len);
            currentColor.r = doc["r"];
            currentColor.g = doc["g"];
            currentColor.b = doc["b"];
            saveConfig();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    webServer.on("/api/brightness", HTTP_POST,
        [](AsyncWebServerRequest *request){},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<64> doc;
            deserializeJson(doc, data, len);
            globalBrightness = doc["brightness"];
            FastLED.setBrightness(globalBrightness);
            saveConfig();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    webServer.on("/api/mode", HTTP_POST,
        [](AsyncWebServerRequest *request){},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<64> doc;
            deserializeJson(doc, data, len);
            currentMode = doc["mode"];
            saveConfig();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    webServer.on("/api/speed", HTTP_POST,
        [](AsyncWebServerRequest *request){},
        NULL,
        [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
            StaticJsonDocument<64> doc;
            deserializeJson(doc, data, len);
            animationSpeed = constrain((int)doc["speed"], 1, 100);
            saveConfig();
            request->send(200, "application/json", "{\"status\":\"ok\"}");
        }
    );

    // Fallback für Root
    webServer.on("/", HTTP_GET, handleRoot);

    // Serve static files from LittleFS (muss am Ende sein!)
    webServer.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    webServer.begin();
    Serial.println("Web server started");
}

void handleRoot(AsyncWebServerRequest *request) {
    // HTML aus PROGMEM senden
    request->send_P(200, "text/html", WEB_UI_HTML);
}

void handleGetStatus(AsyncWebServerRequest *request) {
    StaticJsonDocument<384> doc;
    doc["power"] = powerOn;
    doc["brightness"] = globalBrightness;
    doc["mode"] = currentMode;
    doc["speed"] = animationSpeed;
    doc["color"]["r"] = currentColor.r;
    doc["color"]["g"] = currentColor.g;
    doc["color"]["b"] = currentColor.b;
    doc["geometryLoaded"] = geometryConfigLoaded;
    doc["numHexagons"] = numConfiguredHexagons;

    // Build-Info
    doc["buildDate"] = BUILD_TIMESTAMP;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}

void handlePower(AsyncWebServerRequest *request) {
    powerOn = !powerOn;
    saveConfig();

    StaticJsonDocument<64> doc;
    doc["power"] = powerOn;

    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
}
