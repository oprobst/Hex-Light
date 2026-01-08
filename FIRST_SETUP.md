# Erster Upload - Anleitung

## 1. USB-Upload (einmalig)

Die Datei `platformio.ini` ist bereits für USB-Upload konfiguriert.

### In VS Code:
1. ESP32 per USB anschließen
2. `Ctrl + Alt + U` drücken (Upload)
3. Warten bis "SUCCESS"

## 2. IP-Adresse ablesen

### Serial Monitor öffnen:
```
Ctrl + Alt + S
```

### IP notieren:
```
=== Hex-Light ESP32 Starting ===
LEDs initialized
WiFi connected
IP: 192.168.1.123    ← DIESE IP NOTIEREN!
OTA Update ready!
Hostname: hex-light
=== Setup Complete ===
```

## 3. OTA für zukünftige Uploads aktivieren

### platformio.ini anpassen:

Ersetze diese Zeilen:
```ini
; OTA (Over-The-Air) Upload Configuration
; Für ersten Upload: auskommentieren (USB verwenden)
; upload_protocol = espota
; upload_port = hex-light.local
; Nach erstem Upload: IP-Adresse hier eintragen!
```

Mit:
```ini
; OTA (Over-The-Air) Upload Configuration
upload_protocol = espota
upload_port = 192.168.1.123    ← DEINE IP HIER!
```

## 4. Testen

### Ab jetzt: Upload über WLAN!
```
Ctrl + Alt + U
```

Kein USB-Kabel mehr nötig! 🎉

## Troubleshooting

### "Host Not Found"
- IP-Adresse in `platformio.ini` prüfen
- ESP32 ist im WLAN? (Serial Monitor checken)
- Firewall? (Port 3232 freigeben)

### Hostname statt IP (optional)

**Windows:** Bonjour installieren (kommt mit iTunes)
**Linux/Mac:** Sollte funktionieren

Dann kannst du verwenden:
```ini
upload_port = hex-light.local
```
