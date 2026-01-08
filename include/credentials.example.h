/**
 * WiFi Credentials Template
 *
 * WICHTIG: Diese Datei kopieren nach include/credentials.h und anpassen!
 * include/credentials.h wird durch .gitignore ausgeschlossen.
 */

#ifndef CREDENTIALS_H
#define CREDENTIALS_H

// WiFi Einstellungen
#define WIFI_SSID "DeinWLAN"
#define WIFI_PASSWORD "deinpasswort"

// WiFi Modus
#define USE_AP_MODE false  // false = Station Mode (WLAN-Client), true = Access Point

// OTA (Over-The-Air) Update Passwort (optional, aber empfohlen!)
#define OTA_PASSWORD "dein-ota-passwort"
#define OTA_HOSTNAME "hex-light"

// MQTT Broker Einstellungen
#define MQTT_BROKER "192.168.1.1"        // IP deines MQTT Brokers
#define MQTT_PORT 1883
#define MQTT_USER ""                     // Optional: MQTT Username
#define MQTT_PASSWORD ""                 // Optional: MQTT Passwort
#define MQTT_CLIENT_ID "hex-light"
#define MQTT_TOPIC_PREFIX "hexlight"     // Base Topic

#endif // CREDENTIALS_H
