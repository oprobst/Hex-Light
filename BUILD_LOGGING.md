# Build-Logging einrichten

So kannst du Build-Logs automatisch in eine Datei schreiben lassen.

## Methode 1: VS Code Terminal (einfachste)

### Im VS Code Terminal:

**Linux/Mac/WSL:**
```bash
pio run 2>&1 | tee build.log
```

**Windows (PowerShell):**
```powershell
pio run 2>&1 | Tee-Object -FilePath build.log
```

**Windows (CMD):**
```cmd
pio run > build.log 2>&1
```

## Methode 2: Build-Script verwenden (empfohlen)

Ich habe fertige Scripts erstellt:

### Linux/Mac/WSL:
```bash
# Ausführbar machen (einmalig)
chmod +x scripts/build_with_log.sh

# Verwenden
./scripts/build_with_log.sh
```

### Windows:
```cmd
scripts\build_with_log.bat
```

**Vorteil**: Fügt automatisch Timestamp hinzu und hängt Logs an (statt zu überschreiben).

## Methode 3: VS Code Tasks (fortgeschritten)

Erstelle `.vscode/tasks.json`:

```json
{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "PlatformIO: Build with Log",
            "type": "shell",
            "command": "pio run 2>&1 | tee build.log",
            "group": {
                "kind": "build",
                "isDefault": false
            },
            "presentation": {
                "reveal": "always",
                "panel": "new"
            },
            "problemMatcher": ["$platformio"]
        }
    ]
}
```

Dann: `Ctrl+Shift+P` → "Tasks: Run Task" → "PlatformIO: Build with Log"

## Log-Datei anschauen

```bash
# Ganzes Log
cat build.log

# Nur Fehler
grep -i error build.log

# Nur Warnungen
grep -i warning build.log

# Letzte 50 Zeilen
tail -50 build.log
```

## Log-Datei Location

Die `build.log` wird im Projekt-Root erstellt:
```
Hex-Light/
├── build.log          ← Hier!
├── src/
├── platformio.ini
└── ...
```

**Hinweis**: `build.log` ist bereits in `.gitignore` und wird nicht zu Git hinzugefügt.

## Log automatisch bei jedem Build

Wenn du **immer** Logs schreiben willst, erstelle `.vscode/tasks.json` und überschreibe die Standard-Build-Tasks von PlatformIO.

Oder nutze einfach die Scripts statt der normalen Build-Buttons!

## Tipps

### Log mit Zeitstempel
```bash
pio run 2>&1 | tee "build_$(date +%Y%m%d_%H%M%S).log"
```
Erstellt: `build_20231230_143022.log`

### Nur Fehler loggen
```bash
pio run 2>&1 | grep -i error | tee errors.log
```

### Build + Upload mit Log
```bash
pio run -t upload 2>&1 | tee build.log
```

### Alte Logs löschen
```bash
rm build*.log
```

## Was wird geloggt?

- Compiler-Ausgaben
- Fehler und Warnungen
- Library-Downloads
- Dateigrößen
- Upload-Status
- Alles was im Terminal erscheint!
