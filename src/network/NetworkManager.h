#pragma once

/**
 * Network Manager Modul
 *
 * Verwaltet WiFi-Verbindung und OTA-Updates.
 */

// WiFi Setup (Station oder AP-Modus)
void setupWiFi();

// WiFi Loop Handler (schaltet Fallback-AP nach Timeout ab)
void handleWiFi();

// OTA Update Setup
void setupOTA();

// OTA Loop Handler (muss in loop() aufgerufen werden)
void handleOTA();
