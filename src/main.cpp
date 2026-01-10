#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <Preferences.h>
#include "credentials.h"  // WiFi Credentials (nicht in Git!)

// ===== LED Configuration =====
#define LED_PIN 5              // GPIO Pin für LED Daten
#define NUM_LEDS 238           // Gesamtanzahl LEDs (7 Hexagone × 34 LEDs)
#define LEDS_PER_HEXAGON 34    // 55cm Streifen @ 60 LEDs/m = 34 LEDs (real gemessen)
#define LED_TYPE WS2811        // WS2815 ist kompatibel mit WS2811
#define COLOR_ORDER GRB        // Farbordnung der LEDs

// ===== WiFi Configuration =====
// WiFi Credentials kommen aus credentials.h (nicht in Git!)

// ===== Globale Variablen =====
CRGB leds[NUM_LEDS];
AsyncWebServer server(80);
WiFiClient espClient;
PubSubClient mqttClient(espClient);
Preferences preferences;

// LED Steuerung
uint8_t globalBrightness = 128;
bool powerOn = true;
uint8_t currentMode = 0; // 0=Solid, 1=Rainbow, 2=Pulse, etc.
CRGB currentColor = CRGB::White;
uint8_t animationSpeed = 50; // 1-100, höher = schneller (Standard: 50)

// Hexagon Segmente (7 Hexagone mit jeweils 34 LEDs @ 55cm)
struct HexSegment {
    uint16_t startLED;
    uint16_t count;
    bool enabled;
};

#define NUM_HEXAGONS 7
HexSegment hexagons[NUM_HEXAGONS] = {
    {0, 34, true},      // Hexagon 1: LEDs 0-33
    {34, 34, true},     // Hexagon 2: LEDs 34-67
    {68, 34, true},     // Hexagon 3: LEDs 68-101
    {102, 34, true},    // Hexagon 4: LEDs 102-135
    {136, 34, true},    // Hexagon 5: LEDs 136-169
    {170, 34, true},    // Hexagon 6: LEDs 170-203
    {204, 34, true}     // Hexagon 7: LEDs 204-237
};

// ===== Funktionsdeklarationen =====
void setupWiFi();
void setupOTA();
void setupMQTT();
void setupWebServer();
void setupLEDs();
void updateLEDs();
void handleRoot(AsyncWebServerRequest *request);
void handlePower(AsyncWebServerRequest *request);
void handleGetStatus(AsyncWebServerRequest *request);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttReconnect();
void publishMQTTStatus();
void loadConfig();
void saveConfig();

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
    ArduinoOTA.handle();  // OTA Updates verarbeiten

    // MQTT Verbindung aufrechterhalten
    if (!mqttClient.connected()) {
        mqttReconnect();
    }
    mqttClient.loop();

    updateLEDs();
    delay(20); // 50 FPS
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

// ===== WiFi Setup =====
void setupWiFi() {
    if (USE_AP_MODE) {
        // Access Point Modus
        WiFi.mode(WIFI_AP);
        WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
        Serial.println("WiFi AP Mode started");
        Serial.print("AP IP: ");
        Serial.println(WiFi.softAPIP());
    } else {
        // Station Modus - hier SSID/Passwort anpassen
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
            Serial.println("\nWiFi connection failed - switching to AP mode");
            WiFi.mode(WIFI_AP);
            WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);
        }
    }
}

// ===== OTA Setup =====
void setupOTA() {
    // Hostname setzen
    ArduinoOTA.setHostname(OTA_HOSTNAME);

    // Passwort setzen (optional aber empfohlen!)
    ArduinoOTA.setPassword(OTA_PASSWORD);

    // Port (Standard: 3232)
    ArduinoOTA.setPort(3232);

    // Callbacks für Status-Meldungen
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {  // U_SPIFFS
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

        // Fortschritt auf LEDs anzeigen (optional)
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

// ===== MQTT Setup =====
void setupMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    Serial.print("MQTT Broker: ");
    Serial.println(MQTT_BROKER);
}

// MQTT Callback - wird aufgerufen wenn Message empfangen
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    String message;
    for (unsigned int i = 0; i < length; i++) {
        message += (char)payload[i];
    }

    Serial.print("MQTT Message [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(message);

    String topicStr = String(topic);

    // Power Control
    if (topicStr.endsWith("/power/set")) {
        if (message == "ON") {
            powerOn = true;
        } else if (message == "OFF") {
            powerOn = false;
        }
        saveConfig();
        publishMQTTStatus();
    }

    // Brightness Control (0-255)
    else if (topicStr.endsWith("/brightness/set")) {
        globalBrightness = message.toInt();
        FastLED.setBrightness(globalBrightness);
        saveConfig();
        publishMQTTStatus();
    }

    // Color Control (JSON: {"r":255,"g":0,"b":0})
    else if (topicStr.endsWith("/color/set")) {
        StaticJsonDocument<128> doc;
        DeserializationError error = deserializeJson(doc, message);
        if (!error) {
            currentColor.r = doc["r"];
            currentColor.g = doc["g"];
            currentColor.b = doc["b"];
            saveConfig();
            publishMQTTStatus();
        }
    }

    // Mode Control (0=Solid, 1=Rainbow, 2=Pulse)
    else if (topicStr.endsWith("/mode/set")) {
        currentMode = message.toInt();
        saveConfig();
        publishMQTTStatus();
    }

    // Speed Control (1-100)
    else if (topicStr.endsWith("/speed/set")) {
        animationSpeed = constrain(message.toInt(), 1, 100);
        saveConfig();
        publishMQTTStatus();
    }
}

// MQTT Reconnect mit Auto-Retry
void mqttReconnect() {
    static unsigned long lastRetry = 0;
    unsigned long now = millis();

    // Nur alle 5 Sekunden versuchen
    if (now - lastRetry < 5000) {
        return;
    }
    lastRetry = now;

    Serial.print("MQTT Connecting...");

    // Verbindungsversuch
    bool connected;
    if (strlen(MQTT_USER) > 0) {
        connected = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD);
    } else {
        connected = mqttClient.connect(MQTT_CLIENT_ID);
    }

    if (connected) {
        Serial.println(" connected!");

        // Subscribe zu Command Topics
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/power/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/brightness/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/color/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/mode/set").c_str());
        mqttClient.subscribe((String(MQTT_TOPIC_PREFIX) + "/speed/set").c_str());

        // Initial Status publishen
        publishMQTTStatus();
    } else {
        Serial.print(" failed, rc=");
        Serial.println(mqttClient.state());
    }
}

// MQTT Status publishen
void publishMQTTStatus() {
    if (!mqttClient.connected()) return;

    // Power
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/power").c_str(),
                      powerOn ? "ON" : "OFF", true);

    // Brightness
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/brightness").c_str(),
                      String(globalBrightness).c_str(), true);

    // Mode
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/mode").c_str(),
                      String(currentMode).c_str(), true);

    // Speed
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/speed").c_str(),
                      String(animationSpeed).c_str(), true);

    // Color als JSON
    StaticJsonDocument<128> doc;
    doc["r"] = currentColor.r;
    doc["g"] = currentColor.g;
    doc["b"] = currentColor.b;
    String colorJson;
    serializeJson(doc, colorJson);
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/color").c_str(),
                      colorJson.c_str(), true);

    // IP Address
    String ipAddress;
    if (USE_AP_MODE) {
        ipAddress = WiFi.softAPIP().toString();
    } else {
        ipAddress = WiFi.localIP().toString();
    }
    mqttClient.publish((String(MQTT_TOPIC_PREFIX) + "/ip").c_str(),
                      ipAddress.c_str(), true);
}

// ===== Webserver Setup =====
void setupWebServer() {
    // API Endpoints (müssen VOR serveStatic registriert werden!)
    server.on("/api/status", HTTP_GET, handleGetStatus);
    server.on("/api/power", HTTP_POST, handlePower);

    // POST Endpoints mit JSON Body
    server.on("/api/color", HTTP_POST,
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

    server.on("/api/brightness", HTTP_POST,
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

    server.on("/api/mode", HTTP_POST,
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

    server.on("/api/speed", HTTP_POST,
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
    server.on("/", HTTP_GET, handleRoot);

    // Serve static files from LittleFS (muss am Ende sein!)
    server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");

    server.begin();
    Serial.println("Web server started");
}

// ===== Web Handlers =====
void handleRoot(AsyncWebServerRequest *request) {
    String html = R"HTMLDELIM(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hex-Light Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: #1a1a1a;
            color: #fff;
        }
        h1 { text-align: center; color: #4CAF50; }
        .control { margin: 20px 0; padding: 15px; background: #2a2a2a; border-radius: 8px; }
        button {
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
        }
        button:hover { background: #45a049; }
        input[type="range"] { width: 100%; }
        input[type="color"] { width: 100%; height: 50px; }
        .status { background: #333; padding: 10px; border-radius: 4px; margin: 10px 0; }
    </style>
</head>
<body>
    <h1>🔆 Hex-Light Control</h1>

    <div class="control">
        <h3>Power</h3>
        <button onclick="togglePower()">Toggle Power</button>
        <div class="status">Status: <span id="power-status">ON</span></div>
    </div>

    <div class="control">
        <h3>Color</h3>
        <input type="color" id="colorPicker" value="#ffffff" onchange="setColor()">
    </div>

    <div class="control">
        <h3>Brightness</h3>
        <input type="range" min="0" max="255" value="128" id="brightness" oninput="setBrightness()">
        <div class="status">Value: <span id="brightness-value">128</span></div>
    </div>

    <div class="control">
        <h3>Geschwindigkeit (Animation)</h3>
        <input type="range" min="1" max="100" value="50" id="speed" oninput="setSpeed()">
        <div class="status">Value: <span id="speed-value">50</span></div>
    </div>

    <div class="control">
        <h3>Mode</h3>
        <select id="mode" onchange="setMode()">
            <option value="0">Solid Color</option>
            <option value="1">Rainbow</option>
            <option value="2">Pulse</option>
            <option value="3">Rainbow per Hexagon</option>
            <option value="4">Sequential Blink</option>
            <option value="5">Breathe (Atmen)</option>
            <option value="6">Strobe (Stroboskop)</option>
            <option value="7">Flicker (Flackern)</option>
            <option value="8">Running Light (Einzelne LED)</option>
            <option value="9">Chase (34 LEDs Schweif)</option>
            <option value="10">Strobe Long Off (3x Pause)</option>
            <option value="11">Hexagon Kreislauf (Synchron)</option>
            <option value="12">Hexagon Kreislauf (Komplementär)</option>
        </select>
    </div>

    <script>
        function togglePower() {
            fetch('/api/power', { method: 'POST' })
                .then(r => r.json())
                .then(d => {
                    document.getElementById('power-status').innerText = d.power ? 'ON' : 'OFF';
                });
        }

        function setColor() {
            const color = document.getElementById('colorPicker').value;
            const r = parseInt(color.substr(1,2), 16);
            const g = parseInt(color.substr(3,2), 16);
            const b = parseInt(color.substr(5,2), 16);

            fetch('/api/color', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ r, g, b })
            });
        }

        function setBrightness() {
            const val = document.getElementById('brightness').value;
            document.getElementById('brightness-value').innerText = val;

            fetch('/api/brightness', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ brightness: parseInt(val) })
            });
        }

        function setSpeed() {
            const val = document.getElementById('speed').value;
            document.getElementById('speed-value').innerText = val;

            fetch('/api/speed', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ speed: parseInt(val) })
            });
        }

        function setMode() {
            const mode = document.getElementById('mode').value;

            fetch('/api/mode', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode: parseInt(mode) })
            });
        }

        // Update status periodically
        setInterval(() => {
            fetch('/api/status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('power-status').innerText = d.power ? 'ON' : 'OFF';
                    document.getElementById('brightness').value = d.brightness;
                    document.getElementById('brightness-value').innerText = d.brightness;
                    document.getElementById('speed').value = d.speed;
                    document.getElementById('speed-value').innerText = d.speed;
                });
        }, 2000);
    </script>
</body>
</html>
    )HTMLDELIM";

    request->send(200, "text/html", html);
}

void handleGetStatus(AsyncWebServerRequest *request) {
    StaticJsonDocument<256> doc;
    doc["power"] = powerOn;
    doc["brightness"] = globalBrightness;
    doc["mode"] = currentMode;
    doc["speed"] = animationSpeed;
    doc["color"]["r"] = currentColor.r;
    doc["color"]["g"] = currentColor.g;
    doc["color"]["b"] = currentColor.b;

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


// ===== Config Management =====
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

// ===== LED Update Loop =====
void updateLEDs() {
    if (!powerOn) {
        fill_solid(leds, NUM_LEDS, CRGB::Black);
        FastLED.show();
        return;
    }

    switch (currentMode) {
        case 0: // Solid Color
            fill_solid(leds, NUM_LEDS, currentColor);
            break;

        case 1: // Rainbow
            {
                static uint8_t hue = 0;
                static uint8_t frameCounter = 0;

                fill_rainbow(leds, NUM_LEDS, hue, 7);

                // Geschwindigkeit: höherer Wert = öfter inkrementieren
                frameCounter++;
                if (frameCounter >= (101 - animationSpeed)) {
                    hue++;
                    frameCounter = 0;
                }
            }
            break;

        case 2: // Pulse
            {
                static uint8_t pulseValue = 0;
                static int8_t pulseDirection = 1;

                // Geschwindigkeit beeinflusst die Schrittgröße
                uint8_t stepSize = max(1, animationSpeed / 20);

                pulseValue += pulseDirection * stepSize;
                if (pulseValue >= 255 || pulseValue <= 0) {
                    pulseDirection *= -1;
                    pulseValue = constrain(pulseValue, 0, 255);
                }

                CRGB color = currentColor;
                color.nscale8(pulseValue);
                fill_solid(leds, NUM_LEDS, color);
            }
            break;

        case 3: // Rainbow per Hexagon
            {
                static uint8_t hue = 0;
                static uint8_t frameCounter = 0;
                uint8_t huePerHexagon = 256 / NUM_HEXAGONS;

                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        CRGB hexColor = CHSV(hue + (i * huePerHexagon), 255, 255);
                        for (int j = 0; j < hexagons[i].count; j++) {
                            leds[hexagons[i].startLED + j] = hexColor;
                        }
                    }
                }

                // Geschwindigkeit: höherer Wert = öfter inkrementieren
                frameCounter++;
                if (frameCounter >= (101 - animationSpeed)) {
                    hue++;
                    frameCounter = 0;
                }
            }
            break;

        case 4: // Sequential Blink
            {
                static unsigned long lastChange = 0;
                static int currentHexagon = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 1000ms, 100 = 50ms
                // Formel: delay = 1050 - (animationSpeed * 10)
                unsigned long changeInterval = 1050 - (animationSpeed * 10);

                if (now - lastChange >= changeInterval) {
                    lastChange = now;
                    currentHexagon = (currentHexagon + 1) % NUM_HEXAGONS;
                }

                // Alle LEDs ausschalten
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Nur das aktuelle Hexagon einschalten
                if (hexagons[currentHexagon].enabled) {
                    for (int j = 0; j < hexagons[currentHexagon].count; j++) {
                        leds[hexagons[currentHexagon].startLED + j] = currentColor;
                    }
                }
            }
            break;

        case 5: // Breathe (Atmen)
            {
                static uint8_t breathValue = 128; // Start bei 50% (128/255)
                static int8_t breathDirection = 1;
                static uint8_t frameCounter = 0;

                // Geschwindigkeit beeinflusst wie oft wir den Wert ändern
                // Schneller als vorher (durch Division durch 2)
                frameCounter++;
                uint8_t speedDivisor = max(1, (101 - animationSpeed) / 2);

                if (frameCounter >= speedDivisor) {
                    frameCounter = 0;

                    // Wert zwischen 128 (50%) und 255 (100%)
                    breathValue += breathDirection;
                    if (breathValue >= 255) {
                        breathValue = 255;
                        breathDirection = -1;
                    } else if (breathValue <= 128) {
                        breathValue = 128;
                        breathDirection = 1;
                    }
                }

                // Farbe kopieren und skalieren (50%-100%)
                CRGB scaledColor = currentColor;
                scaledColor.nscale8(breathValue);

                // Alle LEDs mit skalierter Farbe füllen
                fill_solid(leds, NUM_LEDS, scaledColor);
            }
            break;

        case 6: // Strobe (Stroboskop)
            {
                static unsigned long lastStrobe = 0;
                static bool strobeState = false;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 200ms, 100 = 10ms
                // Formel: delay = 210 - (animationSpeed * 2)
                unsigned long strobeInterval = 210 - (animationSpeed * 2);

                if (now - lastStrobe >= strobeInterval) {
                    lastStrobe = now;
                    strobeState = !strobeState;
                }

                if (strobeState) {
                    fill_solid(leds, NUM_LEDS, currentColor);
                } else {
                    fill_solid(leds, NUM_LEDS, CRGB::Black);
                }
            }
            break;

        case 7: // Flicker (Flackern)
            {
                static unsigned long lastFlicker = 0;
                static unsigned long nextFlickerTime = 5000; // Nächster Flacker-Zeitpunkt
                static bool isFlickering = false;
                static uint8_t flickerStep = 0;
                static unsigned long flickerStepTime = 0;
                unsigned long now = millis();

                if (!isFlickering) {
                    // Normales konstantes Licht
                    fill_solid(leds, NUM_LEDS, currentColor);

                    // Prüfen ob es Zeit zum Flackern ist
                    if (now - lastFlicker >= nextFlickerTime) {
                        isFlickering = true;
                        flickerStep = 0;
                        flickerStepTime = now;

                        // Nächstes Flacker-Intervall berechnen
                        // Speed 1 = 10000ms (10 Sek), Speed 100 = 3000ms (3 Sek)
                        nextFlickerTime = 10000 - ((animationSpeed - 1) * 70);
                        lastFlicker = now;
                    }
                } else {
                    // Flacker-Sequenz (schnelle Helligkeitswechsel)
                    if (now - flickerStepTime >= 50) { // Alle 50ms ein Schritt
                        flickerStepTime = now;
                        flickerStep++;

                        // 6 Schritte: Hell-Dunkel-Hell-Dunkel-Hell-Normal
                        switch(flickerStep) {
                            case 1: // Dunkel
                                fill_solid(leds, NUM_LEDS, CRGB::Black);
                                break;
                            case 2: // Hell
                                fill_solid(leds, NUM_LEDS, currentColor);
                                break;
                            case 3: // Dunkel
                                fill_solid(leds, NUM_LEDS, CRGB::Black);
                                break;
                            case 4: // Halb
                                {
                                    CRGB dimColor = currentColor;
                                    dimColor.nscale8(128);
                                    fill_solid(leds, NUM_LEDS, dimColor);
                                }
                                break;
                            case 5: // Hell
                                fill_solid(leds, NUM_LEDS, currentColor);
                                break;
                            default: // Flackern beendet
                                isFlickering = false;
                                fill_solid(leds, NUM_LEDS, currentColor);
                                break;
                        }
                    } else {
                        // Aktuellen Zustand beibehalten während wir auf nächsten Step warten
                        // Nichts zu tun, FastLED.show() wird am Ende aufgerufen
                    }
                }
            }
            break;

        case 8: // Running Light (Einzelne LED)
            {
                static unsigned long lastMove = 0;
                static uint16_t currentLED = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 50ms, 100 = 2.5ms (doppelt so schnell)
                // Formel: delay = (105 - animationSpeed) / 2
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    currentLED = (currentLED + 1) % NUM_LEDS;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Nur die aktuelle LED an
                leds[currentLED] = currentColor;
            }
            break;

        case 9: // Chase/Comet (34 LEDs Schweif)
            {
                static unsigned long lastMove = 0;
                static uint16_t headPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                // Formel: delay = 105 - animationSpeed
                unsigned long moveInterval = 105 - animationSpeed;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    headPosition = (headPosition + 1) % NUM_LEDS;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // 34 LEDs einschalten (von headPosition rückwärts)
                for (int i = 0; i < LEDS_PER_HEXAGON; i++) {
                    int ledPos = headPosition - i;
                    if (ledPos < 0) {
                        ledPos += NUM_LEDS; // Wrap around
                    }
                    leds[ledPos] = currentColor;
                }
            }
            break;

        case 10: // Strobe Long Off (AUS-Phase 9x länger)
            {
                static unsigned long lastChange = 0;
                static bool strobeState = false;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = langsam, 100 = schnell
                // Basis-Intervall für AN-Phase: 1 = 50ms, 100 = 5ms
                unsigned long onInterval = 55 - (animationSpeed / 2);
                unsigned long offInterval = onInterval * 9; // 9x so lang

                unsigned long changeInterval = strobeState ? onInterval : offInterval;

                if (now - lastChange >= changeInterval) {
                    lastChange = now;
                    strobeState = !strobeState;
                }

                if (strobeState) {
                    fill_solid(leds, NUM_LEDS, currentColor);
                } else {
                    fill_solid(leds, NUM_LEDS, CRGB::Black);
                }
            }
            break;

        case 11: // Synchronized Hexagon Running Light
            {
                static unsigned long lastMove = 0;
                static uint8_t currentPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    currentPosition = (currentPosition + 1) % LEDS_PER_HEXAGON;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // In jedem Hexagon die LED an der aktuellen Position einschalten
                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        leds[hexagons[i].startLED + currentPosition] = currentColor;
                    }
                }
            }
            break;

        case 12: // Hexagon Running Light mit Komplementärfarbe
            {
                static unsigned long lastMove = 0;
                static uint8_t currentPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    currentPosition = (currentPosition + 1) % LEDS_PER_HEXAGON;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Komplementärfarbe berechnen (in HSV für echte Farbkreis-Gegensätze)
                CHSV hsvColor = rgb2hsv_approximate(currentColor);
                hsvColor.hue += 128; // 180° im Farbkreis (128 von 256)
                CRGB complementaryColor = hsvColor;

                // Gegenüberliegende Position (34 LEDs / 2 = 17)
                uint8_t oppositePosition = (currentPosition + (LEDS_PER_HEXAGON / 2)) % LEDS_PER_HEXAGON;

                // In jedem Hexagon beide LEDs einschalten
                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        // Hauptfarbe an aktueller Position
                        leds[hexagons[i].startLED + currentPosition] = currentColor;
                        // Komplementärfarbe an gegenüberliegender Position
                        leds[hexagons[i].startLED + oppositePosition] = complementaryColor;
                    }
                }
            }
            break;
    }

    FastLED.show();
}
