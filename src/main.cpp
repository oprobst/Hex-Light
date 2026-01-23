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
#include <cmath>  // Für sin, cos, sqrt, abs (Geometrie-Muster)
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

// ===== Hexagon Geometrie für übergreifende Muster =====
// Jedes Hexagon hat 6 Seiten (1=oben, dann im Uhrzeigersinn bis 6)
// LEDs pro Seite: ~5-6 (34 LEDs / 6 Seiten)
#define LEDS_PER_SIDE 6  // Aufgerundet für Berechnung

// Nachbar-Verbindung
struct HexNeighbor {
    uint8_t side;       // An welcher Seite (1-6)
    uint8_t neighborHex; // Welches Hexagon (0 = kein Nachbar)
};

// Hexagon-Geometrie-Konfiguration
struct HexGeometry {
    uint8_t hexNumber;      // 1-basiert (wie in Config)
    uint8_t startSide;      // Startseite der LEDs (1-6)
    bool clockwise;         // true = Uhrzeigersinn (U), false = gegen (G)
    HexNeighbor neighbors[6]; // Nachbarn pro Seite
    float globalY;          // Berechnete globale Y-Position
    float globalX;          // Berechnete globale X-Position
};

#define MAX_HEXAGONS 12
HexGeometry hexGeometry[MAX_HEXAGONS];
uint8_t numConfiguredHexagons = 0;
bool geometryConfigLoaded = false;

// Y-Koordinaten-Lookup für LED-Positionen (normalisiert 0.0 - 1.0)
// Index 0-33 für die 34 LEDs im Hexagon
float ledGlobalY[NUM_LEDS];
float ledGlobalX[NUM_LEDS];

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

// Hexagon-Geometrie Funktionen
void loadGeometryConfig();
bool parseGeometryConfig(const String& config);
void calculateGlobalPositions();
void calculateLEDCoordinates();
float getLEDLocalY(uint8_t hexIndex, uint8_t ledInHex);
float getLEDLocalX(uint8_t hexIndex, uint8_t ledInHex);
uint8_t getSideForLED(uint8_t ledInHex, uint8_t startSide, bool clockwise);
uint8_t getOffsetToSide1(uint8_t startSide, bool clockwise);
uint8_t getHarmonizedLED(uint8_t hexIndex, uint8_t logicalPos, bool reverseDir = false);
uint8_t getLogicalFromPhysical(uint8_t hexIndex, uint8_t physicalPos);

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
        <div class="status">Geometrie: <span id="geometry-status">?</span></div>
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
            <optgroup label="Basis-Effekte">
                <option value="0">Solid Color</option>
                <option value="1">Rainbow</option>
                <option value="2">Pulse</option>
                <option value="3">Rainbow per Hexagon</option>
                <option value="4">Sequential Blink</option>
                <option value="5">Breathe (Atmen)</option>
                <option value="6">Strobe (Stroboskop)</option>
                <option value="7">Flicker (Flackern)</option>
                <option value="8">Running Light (harmonisiert)</option>
                <option value="9">Chase/Comet (harmonisiert)</option>
                <option value="10">Strobe Long Off (3x Pause)</option>
                <option value="11">Kreislauf Synchron</option>
                <option value="12">Kreislauf Komplementär</option>
                <option value="20">Kreislauf Alternierend</option>
                <option value="21">Kreislauf Alt. + Komplementär</option>
            </optgroup>
            <optgroup label="Debug">
                <option value="22">Debug: Physischer Kreislauf</option>
                <option value="23">Debug: Harmonisierter Kreislauf</option>
            </optgroup>
            <optgroup label="Geometrie-Muster">
                <option value="13">Vertikaler Regenbogen</option>
                <option value="14">Horizontaler Regenbogen</option>
                <option value="15">Vertikaler Farbverlauf</option>
                <option value="16">Vertikale Welle</option>
                <option value="17">Feuer-Effekt</option>
                <option value="18">Aurora/Nordlicht</option>
                <option value="19">Plasma</option>
            </optgroup>
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
                    document.getElementById('geometry-status').innerText =
                        d.geometryLoaded ? ('Geladen (' + d.numHexagons + ' Hex)') : 'NICHT GELADEN';
                    document.getElementById('geometry-status').style.color = d.geometryLoaded ? '#4CAF50' : '#f44336';
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
    doc["geometryLoaded"] = geometryConfigLoaded;
    doc["numHexagons"] = numConfiguredHexagons;

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

// ===== Hexagon-Geometrie Konfiguration =====

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

        case 8: // Running Light (Einzelne LED, harmonisiert über Hexagone)
            {
                static unsigned long lastMove = 0;
                static uint8_t currentHex = 0;        // Aktuelles Hexagon
                static uint8_t logicalPosition = 0;   // Logische Position im Hexagon
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 50ms, 100 = 2.5ms (doppelt so schnell)
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition++;
                    if (logicalPosition >= LEDS_PER_HEXAGON) {
                        logicalPosition = 0;
                        currentHex = (currentHex + 1) % NUM_HEXAGONS;
                    }
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Die aktuelle LED im aktuellen Hexagon einschalten
                if (hexagons[currentHex].enabled) {
                    uint8_t physicalPos = getHarmonizedLED(currentHex, logicalPosition, false);
                    leds[hexagons[currentHex].startLED + physicalPos] = currentColor;
                }
            }
            break;

        case 9: // Chase/Comet (34 LEDs Schweif, harmonisiert)
            {
                static unsigned long lastMove = 0;
                static uint8_t currentHex = 0;
                static uint8_t logicalPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = 105 - animationSpeed;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition++;
                    if (logicalPosition >= LEDS_PER_HEXAGON) {
                        logicalPosition = 0;
                        currentHex = (currentHex + 1) % NUM_HEXAGONS;
                    }
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Schweif: Ein komplettes Hexagon leuchtend (das aktuelle)
                // Plus Fade-out ins vorherige
                int prevHex = (currentHex - 1 + NUM_HEXAGONS) % NUM_HEXAGONS;

                // Aktuelles Hexagon: LEDs von 0 bis logicalPosition leuchten
                for (int i = 0; i <= logicalPosition; i++) {
                    uint8_t physicalPos = getHarmonizedLED(currentHex, i, false);
                    // Helligkeit nimmt ab je weiter hinten
                    uint8_t brightness = 255 - ((logicalPosition - i) * 7);
                    CRGB color = currentColor;
                    color.nscale8(brightness);
                    leds[hexagons[currentHex].startLED + physicalPos] = color;
                }

                // Vorheriges Hexagon: Rest des Schweifs
                int remaining = LEDS_PER_HEXAGON - 1 - logicalPosition;
                for (int i = 0; i < remaining && i < LEDS_PER_HEXAGON; i++) {
                    int pos = LEDS_PER_HEXAGON - 1 - i;
                    uint8_t physicalPos = getHarmonizedLED(prevHex, pos, false);
                    // Helligkeit nimmt weiter ab
                    uint8_t brightness = 255 - ((logicalPosition + 1 + i) * 7);
                    if (brightness > 250) brightness = 0; // Overflow check
                    CRGB color = currentColor;
                    color.nscale8(brightness);
                    leds[hexagons[prevHex].startLED + physicalPos] = color;
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

        case 11: // Synchronized Hexagon Running Light (harmonisiert)
            {
                static unsigned long lastMove = 0;
                static uint8_t logicalPosition = 0;  // Logische Position (0 = oben)
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition = (logicalPosition + 1) % LEDS_PER_HEXAGON;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // In jedem Hexagon die LED an der harmonisierten Position einschalten
                // Alle drehen sich im Uhrzeigersinn, startend an der gleichen Ecke
                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        uint8_t physicalPos = getHarmonizedLED(i, logicalPosition, false);
                        leds[hexagons[i].startLED + physicalPos] = currentColor;
                    }
                }
            }
            break;

        case 12: // Hexagon Running Light mit Komplementärfarbe (harmonisiert)
            {
                static unsigned long lastMove = 0;
                static uint8_t logicalPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition = (logicalPosition + 1) % LEDS_PER_HEXAGON;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Komplementärfarbe berechnen (in HSV für echte Farbkreis-Gegensätze)
                CHSV hsvColor = rgb2hsv_approximate(currentColor);
                hsvColor.hue += 128; // 180° im Farbkreis (128 von 256)
                CRGB complementaryColor = hsvColor;

                // Gegenüberliegende logische Position (34 LEDs / 2 = 17)
                uint8_t oppositeLogical = (logicalPosition + (LEDS_PER_HEXAGON / 2)) % LEDS_PER_HEXAGON;

                // In jedem Hexagon beide LEDs einschalten (harmonisiert)
                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        uint8_t physicalPos = getHarmonizedLED(i, logicalPosition, false);
                        uint8_t physicalOpp = getHarmonizedLED(i, oppositeLogical, false);
                        // Hauptfarbe an aktueller Position
                        leds[hexagons[i].startLED + physicalPos] = currentColor;
                        // Komplementärfarbe an gegenüberliegender Position
                        leds[hexagons[i].startLED + physicalOpp] = complementaryColor;
                    }
                }
            }
            break;

        // ===== Übergreifende Geometrie-Muster (benötigen Konfiguration) =====

        case 13: // Vertikaler Regenbogen (unten nach oben, animiert)
            {
                static uint8_t hueOffset = 0;
                static unsigned long lastUpdate = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms pro Schritt
                unsigned long updateInterval = max(5UL, 105UL - animationSpeed);

                if (now - lastUpdate >= updateInterval) {
                    lastUpdate = now;
                    hueOffset += 2;  // Größere Schritte für sichtbarere Animation
                }

                for (int i = 0; i < NUM_LEDS; i++) {
                    // Y-Koordinate bestimmt den Hue (0.0 = unten = rot, 1.0 = oben = violett)
                    uint8_t hue = (uint8_t)(ledGlobalY[i] * 255) + hueOffset;
                    leds[i] = CHSV(hue, 255, 255);
                }
            }
            break;

        case 14: // Horizontaler Regenbogen (links nach rechts, animiert)
            {
                static uint8_t hueOffset = 0;
                static unsigned long lastUpdate = 0;
                static float minX = 999.0f, maxX = -999.0f;
                static bool initialized = false;
                unsigned long now = millis();

                // Min/Max X nur einmal berechnen
                if (!initialized) {
                    for (int i = 0; i < NUM_LEDS; i++) {
                        if (ledGlobalX[i] < minX) minX = ledGlobalX[i];
                        if (ledGlobalX[i] > maxX) maxX = ledGlobalX[i];
                    }
                    initialized = true;
                }
                float rangeX = maxX - minX;
                if (rangeX < 0.01f) rangeX = 1.0f;

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms pro Schritt
                unsigned long updateInterval = max(5UL, 105UL - animationSpeed);

                if (now - lastUpdate >= updateInterval) {
                    lastUpdate = now;
                    hueOffset += 2;  // Größere Schritte für sichtbarere Animation
                }

                for (int i = 0; i < NUM_LEDS; i++) {
                    float normalizedX = (ledGlobalX[i] - minX) / rangeX;
                    uint8_t hue = (uint8_t)(normalizedX * 255) + hueOffset;
                    leds[i] = CHSV(hue, 255, 255);
                }
            }
            break;

        case 15: // Vertikaler Farbverlauf (currentColor oben, schwarz unten)
            {
                for (int i = 0; i < NUM_LEDS; i++) {
                    // Y = 1.0 (oben) = volle Farbe, Y = 0.0 (unten) = schwarz
                    CRGB color = currentColor;
                    color.nscale8((uint8_t)(ledGlobalY[i] * 255));
                    leds[i] = color;
                }
            }
            break;

        case 16: // Vertikale Welle (Farbe wandert von unten nach oben)
            {
                static float wavePosition = 0.0f;
                static unsigned long lastMove = 0;
                unsigned long now = millis();

                // Geschwindigkeit der Welle
                unsigned long moveInterval = 105 - animationSpeed;
                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    wavePosition += 0.02f;
                    if (wavePosition > 1.5f) wavePosition = -0.5f;
                }

                for (int i = 0; i < NUM_LEDS; i++) {
                    // Berechne Distanz zur Wellenposition
                    float dist = fabs(ledGlobalY[i] - wavePosition);

                    // Welle hat eine Breite von ~0.3
                    if (dist < 0.15f) {
                        // Innerhalb der Welle: volle Farbe
                        uint8_t brightness = (uint8_t)((1.0f - dist / 0.15f) * 255);
                        CRGB color = currentColor;
                        color.nscale8(brightness);
                        leds[i] = color;
                    } else {
                        leds[i] = CRGB::Black;
                    }
                }
            }
            break;

        case 17: // Feuer-Effekt (Flammen von unten)
            {
                static unsigned long lastUpdate = 0;
                unsigned long now = millis();

                unsigned long updateInterval = 105 - animationSpeed;
                if (now - lastUpdate >= updateInterval) {
                    lastUpdate = now;

                    for (int i = 0; i < NUM_LEDS; i++) {
                        // Basis-Helligkeit abhängig von Y (unten = hell, oben = dunkel)
                        float baseHeat = (1.0f - ledGlobalY[i]) * 200;

                        // Zufällige Variation für Flacker-Effekt
                        int heat = (int)baseHeat + random(-30, 30);
                        heat = constrain(heat, 0, 255);

                        // Heat-Palette: schwarz -> rot -> orange -> gelb -> weiß
                        if (heat < 85) {
                            // Schwarz zu rot
                            leds[i] = CRGB(heat * 3, 0, 0);
                        } else if (heat < 170) {
                            // Rot zu orange/gelb
                            leds[i] = CRGB(255, (heat - 85) * 3, 0);
                        } else {
                            // Orange zu weiß
                            leds[i] = CRGB(255, 255, (heat - 170) * 3);
                        }
                    }
                }
            }
            break;

        case 18: // Aurora/Nordlicht (wellenförmige Farbverläufe)
            {
                static float phase = 0.0f;
                static uint8_t frameCounter = 0;

                frameCounter++;
                if (frameCounter >= (101 - animationSpeed) / 2) {
                    phase += 0.05f;
                    frameCounter = 0;
                }

                for (int i = 0; i < NUM_LEDS; i++) {
                    // Mehrere überlagerte Sinuswellen
                    float wave1 = sin(ledGlobalY[i] * 6.28f + phase) * 0.5f + 0.5f;
                    float wave2 = sin(ledGlobalX[i] * 4.0f + phase * 1.3f) * 0.5f + 0.5f;
                    float wave3 = sin((ledGlobalY[i] + ledGlobalX[i]) * 3.0f + phase * 0.7f) * 0.5f + 0.5f;

                    // Kombiniere Wellen für Aurora-Farben (grün/blau/lila)
                    uint8_t hue = 96 + (uint8_t)((wave1 + wave2) * 64); // Grün-Blau Bereich
                    uint8_t brightness = (uint8_t)(wave3 * 200 + 55);

                    leds[i] = CHSV(hue, 255, brightness);
                }
            }
            break;

        case 19: // Plasma (psychedelische Farbmuster)
            {
                static float time = 0.0f;
                static uint8_t frameCounter = 0;

                frameCounter++;
                if (frameCounter >= (101 - animationSpeed) / 3) {
                    time += 0.1f;
                    frameCounter = 0;
                }

                for (int i = 0; i < NUM_LEDS; i++) {
                    float x = ledGlobalX[i];
                    float y = ledGlobalY[i];

                    // Klassischer Plasma-Effekt mit mehreren Sinuswellen
                    float v1 = sin(x * 10.0f + time);
                    float v2 = sin(10.0f * (x * sin(time / 2.0f) + y * cos(time / 3.0f)) + time);
                    float cx = x + 0.5f * sin(time / 5.0f);
                    float cy = y + 0.5f * cos(time / 3.0f);
                    float v3 = sin(sqrt(100.0f * (cx * cx + cy * cy) + 1.0f) + time);

                    float v = (v1 + v2 + v3) / 3.0f;

                    uint8_t hue = (uint8_t)((v + 1.0f) * 127.5f);
                    leds[i] = CHSV(hue, 255, 255);
                }
            }
            break;

        case 20: // Alternierender Kreislauf (jedes 2. Hexagon gegen Uhrzeigersinn)
            {
                static unsigned long lastMove = 0;
                static uint8_t logicalPosition = 0;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition = (logicalPosition + 1) % LEDS_PER_HEXAGON;
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Jedes Hexagon: gerade = im Uhrzeigersinn, ungerade = gegen Uhrzeigersinn
                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        bool reverseDirection = (i % 2 == 1); // Ungerade = gegen Uhrzeigersinn
                        uint8_t physicalPos = getHarmonizedLED(i, logicalPosition, reverseDirection);
                        leds[hexagons[i].startLED + physicalPos] = currentColor;
                    }
                }
            }
            break;

        case 21: // Alternierender Kreislauf mit Komplementärfarbe
            {
                static unsigned long lastMove = 0;
                static uint8_t logicalPosition = 0;
                unsigned long now = millis();

                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                if (now - lastMove >= moveInterval) {
                    lastMove = now;
                    logicalPosition = (logicalPosition + 1) % LEDS_PER_HEXAGON;
                }

                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Komplementärfarbe berechnen
                CHSV hsvColor = rgb2hsv_approximate(currentColor);
                hsvColor.hue += 128;
                CRGB complementaryColor = hsvColor;

                uint8_t oppositeLogical = (logicalPosition + (LEDS_PER_HEXAGON / 2)) % LEDS_PER_HEXAGON;

                for (int i = 0; i < NUM_HEXAGONS; i++) {
                    if (hexagons[i].enabled) {
                        bool reverseDirection = (i % 2 == 1);
                        uint8_t physicalPos = getHarmonizedLED(i, logicalPosition, reverseDirection);
                        uint8_t physicalOpp = getHarmonizedLED(i, oppositeLogical, reverseDirection);

                        leds[hexagons[i].startLED + physicalPos] = currentColor;
                        leds[hexagons[i].startLED + physicalOpp] = complementaryColor;
                    }
                }
            }
            break;

        case 22: // Debug: Physischer Kreislauf (ignoriert Geometrie-Konfiguration)
            {
                // Läuft durch LEDs 0-33 in jedem Hexagon in der physischen Reihenfolge
                // wie sie auf dem Streifen eingeklebt sind (keine Harmonisierung)
                // 1 Sekunde Pause vor LED 0 um den Anfang zu identifizieren
                static unsigned long lastMove = 0;
                static uint8_t physicalPosition = 0;  // Physische Position (0-33)
                static bool pauseBeforeStart = false; // Pause-Flag für LED 0
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                // Pause von 1 Sekunde vor LED 0
                unsigned long currentInterval = pauseBeforeStart ? 1000 : moveInterval;

                if (now - lastMove >= currentInterval) {
                    lastMove = now;

                    if (pauseBeforeStart) {
                        // Pause ist vorbei, jetzt LED 0 anzeigen
                        pauseBeforeStart = false;
                    } else {
                        physicalPosition = (physicalPosition + 1) % LEDS_PER_HEXAGON;

                        // Wenn wir wieder bei 0 sind, nächstes Mal pausieren
                        if (physicalPosition == 0) {
                            pauseBeforeStart = true;
                        }
                    }
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Während der Pause: alle LEDs aus (nichts anzeigen)
                if (!pauseBeforeStart) {
                    // In jedem Hexagon die LED an der physischen Position einschalten
                    // Keine Harmonisierung - direkt LED 0, 1, 2, ... 33
                    for (int i = 0; i < NUM_HEXAGONS; i++) {
                        if (hexagons[i].enabled) {
                            leds[hexagons[i].startLED + physicalPosition] = currentColor;
                        }
                    }
                }
            }
            break;

        case 23: // Debug: Harmonisierter Kreislauf (wie Mode 11, aber mit 1s Pause)
            {
                // Wie Kreislauf Synchron, aber mit 1 Sekunde Pause vor Position 0
                // um zu sehen wo die harmonisierte Startposition (oben) ist
                static unsigned long lastMove = 0;
                static uint8_t logicalPosition = 0;  // Logische Position (0 = oben)
                static bool pauseBeforeStart = false;
                unsigned long now = millis();

                // Geschwindigkeit: 1 = 100ms, 100 = 5ms
                unsigned long moveInterval = (105 - animationSpeed) / 2;
                if (moveInterval < 1) moveInterval = 1;

                // Pause von 1 Sekunde vor Position 0
                unsigned long currentInterval = pauseBeforeStart ? 1000 : moveInterval;

                if (now - lastMove >= currentInterval) {
                    lastMove = now;

                    if (pauseBeforeStart) {
                        pauseBeforeStart = false;
                    } else {
                        logicalPosition = (logicalPosition + 1) % LEDS_PER_HEXAGON;

                        if (logicalPosition == 0) {
                            pauseBeforeStart = true;
                        }
                    }
                }

                // Alle LEDs aus
                fill_solid(leds, NUM_LEDS, CRGB::Black);

                // Während der Pause: alle LEDs aus
                if (!pauseBeforeStart) {
                    // In jedem Hexagon die LED an der harmonisierten Position einschalten
                    for (int i = 0; i < NUM_HEXAGONS; i++) {
                        if (hexagons[i].enabled) {
                            uint8_t physicalPos = getHarmonizedLED(i, logicalPosition, false);
                            leds[hexagons[i].startLED + physicalPos] = currentColor;
                        }
                    }
                }
            }
            break;
    }

    FastLED.show();
}
