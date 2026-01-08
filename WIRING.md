# Verdrahtungsplan - Hex-Light

## Komponenten

- ESP32 Dev Kit C V4
- WS2815 LED-Streifen (12V, 60 LEDs/m)
- 12V Netzteil (min. 90W / 7.5A für volle Konfiguration)
- **LM2596 Step-Down Converter** (12V → 3.3V für ESP32) - **EMPFOHLEN statt AMS1117!**
- Optional: Logic Level Shifter (3.3V → 5V)
- Verbindungskabel

## Schaltplan

```
┌─────────────┐
│   12V PSU   │
└──────┬──────┘
       │
       ├──────────────┐
       │              │
    12V GND        ┌──▼────────────┐
       │           │   WS2815      │
       │           │  LED Strip    │
       │           │               │
       │       ┌───┤ +12V   Data   │
       │       │   │  GND          │
       │       │   └───────────────┘
       │       │         │
       │       │         │ (Signal)
       │       │         │
       │       │   ┌─────▼─────┐
       │       │   │  (Optional)│
       │       │   │   Level    │
       │       │   │  Shifter   │
       │       │   │ 3.3V → 5V  │
       │       │   └─────┬─────┘
       │       │         │
       │       │    ┌────▼────┐
       │       └────┤ VIN     │
       │            │         │
       └────────────┤ GND     │
                    │         │
            ┌───────┤ GPIO 5  │
            │       │ ESP32   │
            │       │ DevKit  │
            │       │ C V4    │
            │       └─────────┘
            │            │
         USB │            │ 5V (optional)
         Power           └─── Bei USB-Power
```

## Detaillierte Verbindungen

### ESP32 zu WS2815

| ESP32 Pin | WS2815 Pin | Kabel | Hinweis |
|-----------|------------|-------|---------|
| GPIO 5    | Data (DI)  | Gelb/Grün | Datensignal |
| GND       | GND        | Schwarz | Gemeinsame Masse **WICHTIG!** |

### Stromversorgung

#### WS2815 LED-Streifen
- **+12V**: Rotes Kabel vom 12V Netzteil
- **GND**: Schwarzes Kabel vom 12V Netzteil

#### ESP32
Option 1 (empfohlen für Entwicklung):
- **USB**: Micro-USB Kabel vom Computer/USB-Netzteil

Option 2 (für Produktion - **EMPFOHLEN: LM2596 Step-Down**):
- **LM2596 Step-Down Converter**: 12V → 3.3V (sehr effizient, bleibt kühl)
- **VIN/3.3V**: Vom LM2596 Ausgang zu ESP32 3.3V Pin
- **GND**: Gemeinsame Masse mit LED-System
- **WICHTIG**: AMS1117 (12V → 3.3V) wird sehr heiß (>50°C) und ist NICHT empfohlen!

## Wichtige Hinweise

### Gemeinsame Masse
**KRITISCH**: ESP32 GND **MUSS** mit LED-Streifen GND verbunden sein!
Ohne gemeinsame Masse funktioniert die Datenübertragung nicht.

### Logikpegel

WS2815 erwarten 5V Datensignal, ESP32 liefert 3.3V.

**In der Praxis funktioniert es oft auch ohne Level Shifter**, wenn:
- Kabel kurz sind (< 1m)
- Gute Stromversorgung
- Erste LEDs direkt beim ESP32

**Bei Problemen**: 3.3V → 5V Level Shifter verwenden:
```
ESP32 GPIO5 → Level Shifter LV → HV → WS2815 Data
     3.3V           3.3V       5V
```

Empfohlene Level Shifter:
- 74HCT125
- 74AHCT125
- SN74HCT245

### Stromversorgung dimensionieren

**LED Spezifikationen (REAL GEMESSEN):**
- WS2815: 12V addressable LEDs
- Pro Hexagon: 55cm Streifen = **34 LEDs** (real gemessen)
- Stromverbrauch pro Hexagon: **315mA @ 12V bei max. Helligkeit** (real gemessen)
- ESP32 + LM2596: ~35mA @ 12V
- Max. Konfiguration: 12 Hexagone = 408 LEDs

**Basierend auf realen Messungen:**

| Konfiguration | Anzahl LEDs | Stromverbrauch (real gemessen) | Empfohlenes Netzteil |
|---------------|-------------|-------------------------------|----------------------|
| 1 Hexagon     | 34 LEDs     | 350mA (ESP32 + LEDs)          | 12V 1A               |
| 3 Hexagone    | 102 LEDs    | ~1.0A                         | 12V 2A               |
| 6 Hexagone    | 204 LEDs    | ~2.0A                         | 12V 3A               |
| 12 Hexagone   | 408 LEDs    | ~4.0A                         | 12V 5A               |

**Berechnungsgrundlage:**
- Pro Hexagon: 315mA @ 12V = 3.78W
- ESP32 + LM2596 Overhead: 35mA @ 12V = 0.42W
- **Sicherheitspuffer einplanen!** Netzteil sollte nicht dauerhaft an der Leistungsgrenze laufen.

### Kabelquerschnitt

Bei langen LED-Streifen Spannungsabfall beachten:

| Länge | Querschnitt | Max. Strom |
|-------|-------------|------------|
| < 2m  | 0.5 mm²     | 3A         |
| < 5m  | 1.0 mm²     | 6A         |
| > 5m  | 1.5 mm²     | 10A        |

**Tipp**: Bei langen Streifen Einspeisung an mehreren Punkten!

### Inbetriebnahme Checkliste

- [ ] 12V Netzteil korrekt angeschlossen (+12V, GND)
- [ ] ESP32 GND mit LED GND verbunden
- [ ] Datenleitung von GPIO 5 zu LED Data
- [ ] Polung geprüft (keine Verpolung!)
- [ ] Netzteil ausreichend dimensioniert
- [ ] Code auf ESP32 geflasht
- [ ] Serial Monitor geöffnet (115200 Baud)

## Optionale Erweiterungen

### Kondensator am LED-Eingang
Zum Schutz: 1000µF Kondensator parallel zur Stromversorgung am LED-Eingang.

### Widerstand am Dateneingang
Signalintegrität: 330Ω Widerstand zwischen ESP32 GPIO5 und LED Data.

### Absicherung
12V Leitung absichern mit passender Sicherung:
- Bis 6 Hexagone (198 LEDs): 5A Sicherung
- 7-12 Hexagone (231-396 LEDs): 10A Sicherung

## Troubleshooting

| Problem | Lösung |
|---------|--------|
| Keine LED leuchtet | GND-Verbindung prüfen, 12V Spannung messen |
| Flackern | Stromversorgung zu schwach, Kabel zu dünn |
| Erste LEDs ok, Rest nicht | Spannungsabfall, Zwischeneinspeisung |
| Falsche Farben | Farbordnung in Code anpassen (RGB/GRB) |
| Sporadische Fehler | Level Shifter verwenden, Kabel kürzen |

## Sicherheitshinweise

- **Netzteil trennen** vor Arbeiten an der Schaltung
- **Polung prüfen** vor dem Einschalten
- **Keine Kurzschlüsse** verursachen
- **Belüftung** bei hoher LED-Anzahl (Wärmeentwicklung)
- **Nicht im Freien** ohne Schutz (IP-Rating beachten)
