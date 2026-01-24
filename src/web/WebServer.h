#pragma once

#include <ESPAsyncWebServer.h>

/**
 * Web Server Modul
 *
 * Stellt das Web-Interface und die REST-API bereit.
 */

// Server-Instanz (definiert in WebServer.cpp)
extern AsyncWebServer webServer;

// Setup-Funktion (aufrufen in setup())
void setupWebServer();

// Handler-Funktionen
void handleRoot(AsyncWebServerRequest *request);
void handleGetStatus(AsyncWebServerRequest *request);
void handlePower(AsyncWebServerRequest *request);
