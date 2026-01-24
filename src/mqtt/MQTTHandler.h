#pragma once

#include <PubSubClient.h>

/**
 * MQTT Handler Modul
 *
 * Verwaltet die MQTT-Kommunikation für Home Assistant Integration.
 */

// MQTT Client (definiert in MQTTHandler.cpp)
extern PubSubClient mqttClient;

// Setup-Funktion (aufrufen in setup())
void setupMQTT();

// Im Loop aufrufen für Reconnect-Logik
void mqttReconnect();

// Status an MQTT publishen
void publishMQTTStatus();

// Callback für eingehende MQTT Messages
void mqttCallback(char* topic, byte* payload, unsigned int length);
