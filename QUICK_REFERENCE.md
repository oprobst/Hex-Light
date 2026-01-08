# Quick Reference - Hex-Light

Schnellreferenz für die wichtigsten Befehle und Einstellungen.

## PlatformIO Shortcuts

| Aktion | Shortcut | Icon | Beschreibung |
|--------|----------|------|--------------|
| Build | `Ctrl+Alt+B` | ✓ | Code kompilieren |
| Upload | `Ctrl+Alt+U` | → | Auf ESP32 laden |
| Serial Monitor | `Ctrl+Alt+S` | 🔌 | Konsole öffnen/schließen |
| Clean | - | 🗑️ | Build-Cache löschen |

## Typischer Workflow

```
1. Code ändern in src/main.cpp
   ↓
2. Speichern (Ctrl+S)
   ↓
3. Build (Ctrl+Alt+B) - Testen ob kompiliert
   ↓
4. Upload (Ctrl+Alt+U) - Auf ESP32 laden
   ↓
5. Serial Monitor (Ctrl+Alt+S) - Ausgabe ansehen
```

## Wichtige Dateien

| Datei | Beschreibung |
|-------|--------------|
| `platformio.ini` | Projekt-Konfiguration, Libraries |
| `src/main.cpp` | Hauptprogramm - hier arbeitest du |
| `include/` | Header-Dateien (.h) |
| `lib/` | Lokale Libraries |
| `.pio/` | Build-Cache (nicht anfassen!) |

## Code-Anpassungen

### WiFi Einstellungen
```cpp
// Zeile 16-18 in main.cpp
const char* WIFI_SSID = "Hex-Light-AP";
const char* WIFI_PASSWORD = "hexlight123";
bool USE_AP_MODE = true;  // true = AP, false = Station
```

### LED Konfiguration
```cpp
// Zeile 10-12 in main.cpp
#define LED_PIN 5              // GPIO Pin
#define NUM_LEDS 396           // Gesamtanzahl LEDs
#define LEDS_PER_HEXAGON 33    // LEDs pro Hexagon
```

### Hexagone anpassen
```cpp
// Zeile 38 in main.cpp
#define NUM_HEXAGONS 12  // Anzahl Hexagone (1-12)

// Wenn du nur 6 Hexagone hast:
#define NUM_HEXAGONS 6
// Dann auch NUM_LEDS anpassen: 6 × 33 = 198
#define NUM_LEDS 198
```

## Problemlösung

| Problem | Lösung |
|---------|--------|
| Upload schlägt fehl | Boot-Button halten während "Connecting..." |
| Serial zeigt Kauderwelsch | Baudrate 115200 prüfen |
| Port in use | Serial Monitor schließen vor Upload |
| LEDs leuchten nicht | GND-Verbindung + 12V prüfen |
| Code kompiliert nicht | Fehlermeldung lesen, Libraries prüfen |

## Serial Monitor Ausgabe

### Normaler Start
```
=== Hex-Light ESP32 Starting ===
LEDs initialized
WiFi AP Mode started
AP IP: 192.168.4.1
Web server started
=== Setup Complete ===
IP Address: 192.168.4.1
```

### WiFi Verbindungsfehler
```
Connecting to WiFi........
WiFi connection failed - switching to AP mode
```

## Webinterface zugreifen

### Access Point Modus
1. WiFi: "Hex-Light-AP" (Passwort: hexlight123)
2. Browser: `http://192.168.4.1`

### Station Modus
1. IP aus Serial Monitor ablesen
2. Browser: `http://<IP-Adresse>`

## API Endpunkte

| Endpunkt | Methode | Body | Beschreibung |
|----------|---------|------|--------------|
| `/api/status` | GET | - | Aktueller Status |
| `/api/power` | POST | - | Power Toggle |
| `/api/color` | POST | `{"r":255,"g":0,"b":0}` | Farbe setzen |
| `/api/brightness` | POST | `{"brightness":200}` | Helligkeit |
| `/api/mode` | POST | `{"mode":1}` | Modus (0=Solid, 1=Rainbow, 2=Pulse) |

## Debugging

### Serial.print() nutzen
```cpp
void loop() {
    Serial.print("Brightness: ");
    Serial.println(globalBrightness);
    delay(1000);
}
```

### LED Test
```cpp
// In setup() nach setupLEDs():
fill_solid(leds, NUM_LEDS, CRGB::Red);
FastLED.show();
delay(2000);  // 2 Sekunden rot
```

## ESP32 Pins (Dev Kit C V4)

| Pin | Nutzung | Hinweis |
|-----|---------|---------|
| GPIO 5 | LED Data | Standard in diesem Projekt |
| GND | Masse | **MUSS** mit LED-Streifen GND verbunden sein |
| 3.3V | Output | Max. 50mA - NICHT für LEDs! |
| 5V | Output | Von USB, für kleine Sensoren |
| VIN | Input | 5V Einspeisung (falls kein USB) |

**Pins zu vermeiden** (interne Nutzung):
- GPIO 0, 2, 15 (Boot-Modus)
- GPIO 6-11 (Flash)

**Empfohlene Pins für LEDs:**
- GPIO 5 (Standard)
- GPIO 16, 17, 18, 19, 21, 22, 23

## Stromversorgung

| Konfiguration | LEDs | Netzteil |
|---------------|------|----------|
| 3 Hexagone | 99 | 12V 3A |
| 6 Hexagone | 198 | 12V 5A |
| 12 Hexagone | 396 | 12V 10A |

**Wichtig**: ESP32 mit 5V (USB) und LEDs mit 12V (separates Netzteil) versorgen!

## Library Versionen

Siehe `platformio.ini`:
- FastLED: ^3.6.0
- ArduinoJson: ^6.21.3
- ESPAsyncWebServer: GitHub
- AsyncTCP: GitHub

## Nützliche Links

- **PlatformIO Docs**: https://docs.platformio.org/
- **ESP32 Pinout**: https://randomnerdtutorials.com/esp32-pinout-reference-gpios/
- **FastLED Wiki**: https://github.com/FastLED/FastLED/wiki
- **WS2815 Datasheet**: Google "WS2815 datasheet"

## Befehle (Terminal)

```bash
# Build
pio run

# Upload
pio run -t upload

# Serial Monitor
pio device monitor

# Upload + Monitor
pio run -t upload && pio device monitor

# Clean
pio run -t clean

# Erase Flash komplett
pio run -t erase
```

## Git Workflow (für später)

```bash
# Status prüfen
git status

# Änderungen hinzufügen
git add .

# Commit erstellen
git commit -m "Beschreibung der Änderung"

# Zu GitHub pushen
git push origin main
```

---

**Tipp**: Diese Datei ausdrucken oder als Browser-Tab offen lassen beim Entwickeln!
