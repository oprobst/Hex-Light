# Hex-Light - ESP32 LED Wandlampen Controller

WiFi-gesteuertes Beleuchtungssystem für eine hexagonale Wandlampe mit WS2815 LED-Streifen.

**Neu mit PlatformIO & ESP32?** → Siehe [GETTING_STARTED.md](GETTING_STARTED.md) für eine ausführliche Einsteiger-Anleitung!
**Schnellreferenz** → [QUICK_REFERENCE.md](QUICK_REFERENCE.md)

## Hardware

- **Mikrocontroller**: ESP32 Dev Kit C V4
- **LED-Streifen**: WS2815 (12V, 60 LEDs/m)
- **Stromversorgung**: 12V DC + LM2596 Step-Down (12V → 3.3V für ESP32)
- **LED-Layout**: Bis zu 12 Hexagone mit je 34 LEDs (55cm Streifen pro Hexagon)
- **Stromverbrauch**: 315mA @ 12V pro Hexagon bei max. Helligkeit (real gemessen)
- **Max. Konfiguration**: 12 Hexagone = 408 LEDs gesamt

## Features

- Web-basierte Steuerung über WiFi
- Access Point oder Station Modus
- Ansteuerung einzelner oder aller Hexagone
- Farb- und Helligkeitssteuerung
- Verschiedene Anzeigemodi:
  - Solid Color (einfarbig)
  - Rainbow (Regenbogen)
  - Pulse (Pulsieren)
- JSON REST API
- Responsive Web Interface

## Pin-Belegung

| Funktion | GPIO Pin | Bemerkung |
|----------|----------|-----------|
| LED Data | GPIO 5   | Signal zu WS2815 |
| GND      | GND      | Gemeinsame Masse |
| Power    | -        | 12V externe Versorgung |

**Wichtig**: ESP32 und LED-Streifen müssen eine gemeinsame Masse (GND) haben!

## Installation

### Voraussetzungen

- [Visual Studio Code](https://code.visualstudio.com/)
- [PlatformIO Extension](https://platformio.org/install/ide?install=vscode)

### Setup

1. Repository klonen:
```bash
git clone <repository-url>
cd Hex-Light
```

2. Projekt in VS Code öffnen:
```bash
code .
```

3. PlatformIO: "Build" ausführen (Ctrl+Alt+B)

4. Konfiguration anpassen in `src/main.cpp`:
```cpp
// WiFi Modus
bool USE_AP_MODE = true;  // true = Access Point, false = Station

// Access Point Modus
const char* WIFI_SSID = "Hex-Light-AP";
const char* WIFI_PASSWORD = "hexlight123";

// LED Konfiguration
#define LED_PIN 5
#define NUM_LEDS 408           // 12 Hexagone × 34 LEDs
#define LEDS_PER_HEXAGON 34    // 55cm @ 60 LEDs/m (real gemessen)

// Hexagon Segmente anpassen (max. 12 Hexagone)
HexSegment hexagons[NUM_HEXAGONS] = {
    {0, 34, true},      // Hexagon 1: Start bei LED 0, 34 LEDs
    {34, 34, true},     // Hexagon 2: Start bei LED 34, 34 LEDs
    // ... bis zu 12 Hexagone möglich
};
```

5. Upload zum ESP32:
   - ESP32 per USB verbinden
   - PlatformIO: "Upload" (Ctrl+Alt+U)

## Verwendung

### WiFi Verbindung

**Access Point Modus** (Standard):
1. ESP32 startet als Access Point "Hex-Light-AP"
2. Mit dem WiFi "Hex-Light-AP" verbinden (Passwort: `hexlight123`)
3. Browser öffnen: `http://192.168.4.1`

**Station Modus**:
1. WIFI_SSID und WIFI_PASSWORD in main.cpp anpassen
2. `USE_AP_MODE = false` setzen
3. Nach Upload: IP-Adresse im Serial Monitor ablesen
4. Browser öffnen: `http://<IP-Adresse>`

### Web Interface

Das Web Interface bietet folgende Steuerungsmöglichkeiten:
- **Power**: Ein/Ausschalten
- **Color**: Farbwahl über Color Picker
- **Brightness**: Helligkeit (0-255)
- **Mode**: Anzeigemodus auswählen

### API Endpunkte

| Endpunkt | Methode | Parameter | Beschreibung |
|----------|---------|-----------|--------------|
| `/api/status` | GET | - | Aktueller Status |
| `/api/power` | POST | - | Power Toggle |
| `/api/color` | POST | `{r, g, b}` | Farbe setzen |
| `/api/brightness` | POST | `{brightness}` | Helligkeit setzen |
| `/api/mode` | POST | `{mode}` | Modus setzen |

### Beispiel API Calls

```bash
# Status abfragen
curl http://192.168.4.1/api/status

# Farbe setzen (Rot)
curl -X POST http://192.168.4.1/api/color \
  -H "Content-Type: application/json" \
  -d '{"r":255,"g":0,"b":0}'

# Helligkeit setzen
curl -X POST http://192.168.4.1/api/brightness \
  -H "Content-Type: application/json" \
  -d '{"brightness":200}'
```

## Entwicklung

### Projektstruktur

```
Hex-Light/
├── src/
│   └── main.cpp          # Hauptprogramm
├── include/              # Header-Dateien
├── lib/                  # Lokale Bibliotheken
├── data/                 # Filesystem-Daten (optional)
├── platformio.ini        # PlatformIO Konfiguration
└── README.md
```

### Erweiterungen

#### Neue Anzeigemodi hinzufügen

In `src/main.cpp` in der Funktion `updateLEDs()`:

```cpp
case 3: // Neuer Modus
    // Deine LED Animation
    break;
```

#### Hexagon-spezifische Steuerung

Einzelne Hexagone ansprechen:

```cpp
// Hexagon 0 rot färben
for (int i = hexagons[0].startLED; i < hexagons[0].startLED + hexagons[0].count; i++) {
    leds[i] = CRGB::Red;
}
FastLED.show();
```

## Fehlerbehebung

### LED-Streifen leuchtet nicht
- 12V Versorgung prüfen
- Datenleitung korrekt angeschlossen? (GPIO 5)
- Gemeinsame Masse zwischen ESP32 und LED-Streifen?

### Keine WiFi-Verbindung
- Serial Monitor öffnen (115200 Baud)
- IP-Adresse ablesen
- Bei Station Modus: SSID/Passwort korrekt?

### Upload schlägt fehl
- USB-Kabel prüfen
- Richtigen COM-Port gewählt?
- Boot-Button während Upload gedrückt halten

## Stromversorgung

**Wichtig**: WS2815 sind 12V LEDs!

### ESP32 Versorgung
- **Entwicklung**: 5V über USB
- **Produktion**: **LM2596 Step-Down Converter** (12V → 3.3V) - **EMPFOHLEN!**
- **NICHT verwenden**: AMS1117 bei 12V Eingang (wird sehr heiß >50°C, ineffizient)

### LED-Streifen Versorgung
- 12V Netzteil (siehe Tabelle unten)
- **Real gemessen**: 315mA @ 12V pro Hexagon bei max. Helligkeit
- ESP32 + LM2596 Overhead: ~35mA @ 12V

**Strombedarfsberechnung (basierend auf realen Messungen):**

| Anzahl Hexagone | Anzahl LEDs | Stromverbrauch | Leistung | Empfohlenes Netzteil |
|-----------------|-------------|----------------|----------|----------------------|
| 1 Hexagon       | 34 LEDs     | 350mA          | 4.2W     | 12V 1A               |
| 3 Hexagone      | 102 LEDs    | ~1.0A          | 12W      | 12V 2A               |
| 6 Hexagone      | 204 LEDs    | ~2.0A          | 24W      | 12V 3A               |
| 12 Hexagone     | 408 LEDs    | ~4.0A          | 48W      | 12V 5A               |

**Berechnungsgrundlage:**
- Pro Hexagon: 315mA @ 12V = 3.78W
- Immer Sicherheitspuffer einplanen!

## Libraries

- [FastLED](https://github.com/FastLED/FastLED) - LED Ansteuerung
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer) - Async Webserver
- [ArduinoJson](https://arduinojson.org/) - JSON Handling

## Lizenz

MIT License

## Autor

Erstellt für hexagonale Wandlampe mit WS2815 LED-Streifen
