# Getting Started - PlatformIO & ESP32

Eine Schritt-für-Schritt-Anleitung für Einsteiger.

## 1. Voraussetzungen installieren

### Visual Studio Code
1. Download von [code.visualstudio.com](https://code.visualstudio.com/)
2. Installieren und starten

### PlatformIO Extension
1. In VS Code: Linke Sidebar → Extensions (oder `Ctrl+Shift+X`)
2. Suche nach "PlatformIO IDE"
3. Auf "Install" klicken
4. Warten bis Installation abgeschlossen (kann 2-5 Minuten dauern)
5. VS Code neu starten

**Wichtig**: PlatformIO lädt beim ersten Start zusätzliche Tools herunter. Das kann beim ersten Mal etwas länger dauern!

## 2. Projekt öffnen

### Projekt in VS Code laden
1. VS Code starten
2. `File` → `Open Folder` (oder `Ctrl+K Ctrl+O`)
3. Navigiere zum `Hex-Light` Ordner
4. Klick auf "Ordner auswählen"

### PlatformIO erkennt das Projekt automatisch
- PlatformIO sieht die `platformio.ini` und lädt alle benötigten Libraries
- Unten in der Statusleiste erscheinen PlatformIO-Icons
- Beim ersten Öffnen werden die Dependencies heruntergeladen (dauert 1-2 Min)

## 3. PlatformIO Benutzeroberfläche

### Statusleiste (unten)
Die wichtigsten Icons von links nach rechts:

```
🏠 Home    ✓ Build    → Upload    🗑️ Clean    🔌 Serial Monitor    📋 Tasks
```

| Icon | Funktion | Shortcut | Beschreibung |
|------|----------|----------|--------------|
| 🏠 | PlatformIO Home | - | Startseite von PlatformIO |
| ✓ | Build | `Ctrl+Alt+B` | Code kompilieren |
| → | Upload | `Ctrl+Alt+U` | Code auf ESP32 laden |
| 🗑️ | Clean | - | Build-Verzeichnis löschen |
| 🔌 | Serial Monitor | `Ctrl+Alt+S` | Serielle Konsole öffnen |
| 📋 | Tasks | - | Alle verfügbaren Tasks anzeigen |

### Linke Sidebar
- **PlatformIO Icon** (Alien-Kopf): Öffnet PlatformIO Menü
  - Project Tasks
  - Libraries
  - Boards
  - Devices (zeigt verbundene ESP32)

## 4. ESP32 anschließen

### Hardware vorbereiten
1. **ESP32 Dev Kit C V4** per Micro-USB-Kabel an PC anschließen
2. **Treiber prüfen**:
   - Windows: Meist automatisch, ggf. CP210x oder CH340 Treiber installieren
   - Linux: Meist keine Treiber nötig
   - Mac: Meist automatisch

### Verbindung prüfen
1. In VS Code: PlatformIO Icon (linke Sidebar) → `Devices`
2. Dein ESP32 sollte dort erscheinen (z.B. `COM3` unter Windows, `/dev/ttyUSB0` unter Linux)

**Wenn ESP32 nicht erkannt wird:**
- Anderes USB-Kabel probieren (manche sind nur zum Laden!)
- Treiber für den USB-Chip installieren:
  - CP210x: [Silicon Labs](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
  - CH340: [CH340 Treiber](http://www.wch-ic.com/downloads/CH341SER_EXE.html)

## 5. Erster Upload - Schritt für Schritt

### Schritt 1: Code kompilieren (Build)
1. Klick auf **✓** (Build) Icon in der Statusleiste
   - ODER: `Ctrl+Alt+B`
   - ODER: PlatformIO Sidebar → PROJECT TASKS → esp32dev → General → Build

**Was passiert:**
- PlatformIO lädt alle Libraries (beim ersten Mal!)
- Kompiliert den Code in `src/main.cpp`
- Zeigt Fortschritt im Terminal unten

**Erfolgreich wenn:**
```
SUCCESS: Took X.XX seconds
```

**Bei Fehlern:**
- Fehlermeldung lesen (meist in rot)
- Oft sind es fehlende Libraries (werden normalerweise automatisch geladen)

### Schritt 2: Code auf ESP32 hochladen (Upload)
1. ESP32 per USB verbunden?
2. Klick auf **→** (Upload) Icon in der Statusleiste
   - ODER: `Ctrl+Alt+U`

**Was passiert:**
- Code wird kompiliert (falls nötig)
- ESP32 wird in den Upload-Modus versetzt
- Firmware wird übertragen
- ESP32 startet neu

**Erfolgreich wenn:**
```
Leaving...
Hard resetting via RTS pin...
SUCCESS
```

**Wenn Upload fehlschlägt:**
```
A fatal error occurred: Failed to connect
```
→ **Lösung**:
1. Boot-Button am ESP32 gedrückt halten
2. Upload erneut starten
3. Wenn "Connecting..." erscheint, Boot-Button loslassen

### Schritt 3: Serial Monitor öffnen
1. Klick auf **🔌** (Serial Monitor) Icon
   - ODER: `Ctrl+Alt+S`

**Was du siehst:**
```
=== Hex-Light ESP32 Starting ===
LEDs initialized
WiFi AP Mode started
AP IP: 192.168.4.1
Web server started
=== Setup Complete ===
IP Address: 192.168.4.1
```

**Serial Monitor schließen:**
- Klick erneut auf 🔌 Icon
- ODER: Im Terminal auf Mülleimer-Icon

## 6. Code bearbeiten

### Datei öffnen
1. Links in der Explorer-Sidebar: `src/main.cpp` öffnen
2. Code anpassen (z.B. WiFi SSID/Passwort)

### Typischer Workflow
1. **Code ändern** in `src/main.cpp`
2. **Speichern** (`Ctrl+S`)
3. **Build** (`Ctrl+Alt+B`) - zum Testen ob Code kompiliert
4. **Upload** (`Ctrl+Alt+U`) - zum Hochladen auf ESP32
5. **Serial Monitor** (`Ctrl+Alt+S`) - zum Debuggen

### Wichtige Code-Stellen zum Anpassen

```cpp
// WiFi Konfiguration (Zeile 16-18)
const char* WIFI_SSID = "Hex-Light-AP";        // ← SSID ändern
const char* WIFI_PASSWORD = "hexlight123";     // ← Passwort ändern
bool USE_AP_MODE = true;                       // ← false für Station Mode

// LED Konfiguration (Zeile 10-13)
#define LED_PIN 5              // ← GPIO Pin für LEDs
#define NUM_LEDS 396           // ← Anzahl LEDs anpassen
#define LEDS_PER_HEXAGON 33    // ← LEDs pro Hexagon

// Anzahl Hexagone (Zeile 38)
#define NUM_HEXAGONS 12        // ← Wenn du weniger hast, hier anpassen
```

## 7. Webinterface testen

### Access Point Modus (Standard)
1. ESP32 startet als WiFi Access Point
2. Auf deinem Handy/PC: WiFi-Einstellungen öffnen
3. Netzwerk "Hex-Light-AP" suchen
4. Verbinden mit Passwort: `hexlight123`
5. Browser öffnen: `http://192.168.4.1`
6. Du siehst das Hex-Light Control Interface!

### Station Modus (Heimnetzwerk)
1. In `main.cpp` ändern:
   ```cpp
   bool USE_AP_MODE = false;
   const char* WIFI_SSID = "DeinWLAN";
   const char* WIFI_PASSWORD = "deinpasswort";
   ```
2. Upload auf ESP32
3. Serial Monitor öffnen → IP-Adresse ablesen
4. Browser: `http://<die-angezeigte-ip>`

## 8. Häufige Probleme & Lösungen

### Build Fehler: "Library not found"
**Problem**: Eine Library fehlt
**Lösung**:
- PlatformIO lädt Libraries automatisch aus `platformio.ini`
- Warten bis Download abgeschlossen
- Falls nicht: PlatformIO Home → Libraries → Suchen & Installieren

### Upload Fehler: "Failed to connect"
**Problem**: ESP32 nicht im Upload-Modus
**Lösung**:
1. Boot-Button am ESP32 drücken und halten
2. Upload starten
3. Bei "Connecting..." Boot-Button loslassen

### Serial Monitor zeigt Kauderwelsch
**Problem**: Falsche Baudrate
**Lösung**:
- In `platformio.ini` steht: `monitor_speed = 115200`
- Serial Monitor sollte automatisch 115200 Baud verwenden
- Falls nicht: Serial Monitor schließen und neu öffnen

### "Port already in use"
**Problem**: Serial Monitor noch offen beim Upload
**Lösung**: Serial Monitor schließen vor Upload

### LEDs leuchten nicht
**Problem**: Hardware-Verbindung
**Lösung**:
1. 12V Netzteil angeschlossen?
2. GND verbunden zwischen ESP32 und LED-Streifen?
3. Datenleitung an GPIO 5?
4. Serial Monitor öffnen - sieht du "LEDs initialized"?

### ESP32 reagiert nicht mehr
**Problem**: Abstürze oder hängt
**Lösung**:
1. USB-Kabel abziehen
2. 5 Sekunden warten
3. USB-Kabel wieder anstecken
4. Neu uploaden

## 9. Nützliche Tipps

### IntelliSense (Code-Vervollständigung)
- PlatformIO konfiguriert automatisch IntelliSense
- Beim Tippen erscheinen Vorschläge
- `Ctrl+Space` für manuelle Vorschläge

### Fehler finden
- Rote Wellenlinien = Fehler
- Gelbe Wellenlinien = Warnungen
- Hover über Fehler für Details

### Terminal-Shortcuts
- `Ctrl+C` im Terminal = Laufenden Prozess abbrechen
- Mehrere Terminals möglich (z.B. Serial Monitor + Build)

### PlatformIO Tasks
- Klick auf 📋 (Tasks) in Statusleiste
- Zeigt alle verfügbaren Aktionen:
  - Build
  - Upload
  - Clean
  - Monitor
  - Upload and Monitor (kombiniert!)
  - Erase Flash (ESP32 komplett löschen)

### Schneller Workflow: Upload & Monitor
Statt Upload → Serial Monitor einzeln:
1. PlatformIO Sidebar → PROJECT TASKS → esp32dev → General
2. **"Upload and Monitor"** - lädt hoch und öffnet sofort Serial Monitor!

## 10. Projekt erweitern

### Neue Datei hinzufügen
1. Rechtsklick auf `src/` → New File
2. Dateiname eingeben (z.B. `effects.cpp`)
3. In `src/main.cpp` einbinden:
   ```cpp
   #include "effects.h"
   ```

### Externe Library hinzufügen
1. Auf [platformio.org/lib](https://platformio.org/lib) suchen
2. In `platformio.ini` unter `lib_deps` hinzufügen:
   ```ini
   lib_deps =
       fastled/FastLED@^3.6.0
       deine-neue-library
   ```
3. Speichern → PlatformIO lädt automatisch herunter

### Debugging mit Serial.print()
```cpp
void setup() {
    Serial.begin(115200);
    Serial.println("Starte Setup...");

    // Dein Code
    Serial.print("LED_PIN ist: ");
    Serial.println(LED_PIN);
}
```

## 11. Nützliche Ressourcen

### Dokumentation
- [PlatformIO Docs](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [FastLED Dokumentation](https://github.com/FastLED/FastLED/wiki)

### Befehle in der Kommandozeile (optional)
Du kannst auch das eingebaute Terminal nutzen:
```bash
# Build
pio run

# Upload
pio run -t upload

# Serial Monitor
pio device monitor

# Clean
pio run -t clean
```

## 12. Checkliste vor dem ersten Upload

- [ ] PlatformIO Extension installiert
- [ ] Projekt in VS Code geöffnet
- [ ] ESP32 per USB verbunden
- [ ] ESP32 in Devices sichtbar
- [ ] Code erfolgreich kompiliert (Build ✓)
- [ ] WiFi SSID/Passwort angepasst (optional)
- [ ] LED_PIN korrekt (GPIO 5)
- [ ] NUM_LEDS angepasst (passend zu deiner Hexagon-Anzahl)

## Viel Erfolg!

Bei Fragen: Serial Monitor ist dein bester Freund zum Debuggen!
