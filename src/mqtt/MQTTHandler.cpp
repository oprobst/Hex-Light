#include "MQTTHandler.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include "credentials.h"

// Externe Variablen aus main.cpp
extern bool powerOn;
extern uint8_t globalBrightness;
extern uint8_t currentMode;
extern CRGB currentColor;
extern uint8_t animationSpeed;
extern void saveConfig();

// WiFi Client für MQTT
WiFiClient mqttWifiClient;
PubSubClient mqttClient(mqttWifiClient);

void setupMQTT() {
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);

    Serial.print("MQTT Broker: ");
    Serial.println(MQTT_BROKER);
}

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
