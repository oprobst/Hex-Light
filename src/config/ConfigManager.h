#pragma once

/**
 * Config Manager Modul
 *
 * Lädt und speichert Einstellungen in den ESP32 Preferences (NVS).
 */

// Config laden (aufrufen in setup())
void loadConfig();

// Config speichern
void saveConfig();
