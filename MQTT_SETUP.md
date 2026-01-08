# MQTT Broker Setup mit Docker

## Mosquitto MQTT Broker starten

Der MQTT Broker läuft in einem Docker Container und vermittelt die Kommunikation zwischen dem ESP32 und OpenHAB.

### Voraussetzungen
- Docker installiert
- Docker Compose installiert

### Broker starten

```bash
# Im Projekt-Verzeichnis:
docker-compose up -d

# Logs anzeigen:
docker-compose logs -f mosquitto

# Status prüfen:
docker-compose ps
```

### Broker stoppen

```bash
docker-compose down
```

### MQTT Broker testen

```bash
# Mit mosquitto_pub/sub (falls installiert):
# Subscribe (Terminal 1):
mosquitto_sub -h 192.168.3.1 -t "hexlight/#" -v

# Publish (Terminal 2):
mosquitto_pub -h 192.168.3.1 -t "hexlight/test" -m "Hello MQTT"
```

### Konfiguration

**Ports:**
- `1883` - Standard MQTT Port (unverschlüsselt)
- `9001` - WebSocket Port für Browser-basierte Clients

**Authentifizierung:**
- Aktuell: `allow_anonymous true` (keine Passwörter erforderlich)
- Für Produktion: Passwort-Authentifizierung in `mosquitto/config/mosquitto.conf` aktivieren

**Daten-Verzeichnisse:**
- `mosquitto/config/` - Konfigurationsdateien
- `mosquitto/data/` - Persistente MQTT Nachrichten
- `mosquitto/log/` - Log-Dateien

### ESP32 Verbindung

Der ESP32 verbindet sich automatisch zum MQTT Broker bei:
- **Host:** 192.168.3.1 (konfiguriert in `include/credentials.h`)
- **Port:** 1883
- **Client ID:** hex-light
- **Topics:**
  - `hexlight/power` (Status: ON/OFF)
  - `hexlight/power/set` (Command)
  - `hexlight/brightness` (Status: 0-255)
  - `hexlight/brightness/set` (Command)
  - `hexlight/color` (Status: JSON `{"r":255,"g":0,"b":0}`)
  - `hexlight/color/set` (Command)
  - `hexlight/mode` (Status: 0=Solid, 1=Rainbow, 2=Pulse)
  - `hexlight/mode/set` (Command)

### OpenHAB Integration

Die OpenHAB Konfiguration ist in `openhab/hexlight.things` vorbereitet.

**WICHTIG:** In der `openhab/hexlight.things` muss die Broker-IP auf die IP des Docker-Hosts gesetzt werden (nicht 192.168.3.1, wenn OpenHAB auf einem anderen Host läuft).

### Troubleshooting

**Broker startet nicht:**
```bash
# Logs prüfen:
docker-compose logs mosquitto

# Container neu starten:
docker-compose restart mosquitto
```

**ESP32 kann sich nicht verbinden:**
- Prüfe ob der Broker läuft: `docker-compose ps`
- Prüfe die Firewall: Port 1883 muss offen sein
- Prüfe die IP in `include/credentials.h`
