#pragma once

/**
 * Network Manager Modul
 *
 * Verwaltet WiFi-Verbindung und OTA-Updates.
 */

// WiFi Setup (Station oder AP-Modus)
void setupWiFi();

// OTA Update Setup
void setupOTA();

// OTA Loop Handler (muss in loop() aufgerufen werden)
void handleOTA();
