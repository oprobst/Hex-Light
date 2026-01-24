#pragma once

#include <Arduino.h>

// HTML UI als PROGMEM String (spart RAM)
const char WEB_UI_HTML[] PROGMEM = R"HTMLDELIM(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Hex-Light Control</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            max-width: 600px;
            margin: 50px auto;
            padding: 20px;
            background: #1a1a1a;
            color: #fff;
        }
        h1 { text-align: center; color: #4CAF50; }
        .control { margin: 20px 0; padding: 15px; background: #2a2a2a; border-radius: 8px; }
        button {
            padding: 10px 20px;
            font-size: 16px;
            cursor: pointer;
            background: #4CAF50;
            color: white;
            border: none;
            border-radius: 4px;
        }
        button:hover { background: #45a049; }
        input[type="range"] { width: 100%; }
        input[type="color"] { width: 100%; height: 50px; }
        .status { background: #333; padding: 10px; border-radius: 4px; margin: 10px 0; }

        /* Mode Categories */
        .mode-category { margin: 8px 0; }
        .category-header {
            background: #3a3a3a;
            padding: 10px 15px;
            border-radius: 6px;
            cursor: pointer;
            user-select: none;
            display: flex;
            align-items: center;
            gap: 8px;
        }
        .category-header:hover { background: #444; }
        .arrow { transition: transform 0.2s; font-size: 0.8em; }
        .arrow.open { transform: rotate(90deg); }
        .category-content {
            display: flex;
            flex-wrap: wrap;
            gap: 6px;
            padding: 10px;
            background: #252525;
            border-radius: 0 0 6px 6px;
            margin-top: -2px;
        }
        .mode-btn {
            padding: 8px 12px;
            font-size: 13px;
            background: #444;
            border: 2px solid transparent;
            border-radius: 4px;
            cursor: pointer;
            transition: all 0.15s;
        }
        .mode-btn:hover { background: #555; }
        .mode-btn.active {
            background: #4CAF50;
            border-color: #6fcf73;
        }
    </style>
</head>
<body>
    <h1>Hex-Light Control</h1>

    <div class="control">
        <h3>Power</h3>
        <button onclick="togglePower()">Toggle Power</button>
        <div class="status">Status: <span id="power-status">ON</span></div>
        <div class="status">Geometrie: <span id="geometry-status">?</span></div>
        <div class="status" style="font-size:0.85em; color:#888;">
            Build: <span id="build-info">...</span>
        </div>
    </div>

    <div class="control">
        <h3>Color</h3>
        <input type="color" id="colorPicker" value="#ffffff" onchange="setColor()">
    </div>

    <div class="control">
        <h3>Brightness</h3>
        <input type="range" min="0" max="255" value="128" id="brightness" oninput="setBrightness()">
        <div class="status">Value: <span id="brightness-value">128</span></div>
    </div>

    <div class="control">
        <h3>Geschwindigkeit (Animation)</h3>
        <input type="range" min="1" max="100" value="50" id="speed" oninput="setSpeed()">
        <div class="status">Value: <span id="speed-value">50</span></div>
    </div>

    <div class="control">
        <h3>Effekt <span id="current-mode-name" style="color:#4CAF50; font-size:0.8em;">(Solid Color)</span></h3>

        <div class="mode-category">
            <div class="category-header" onclick="toggleCategory('basis')">
                <span class="arrow" id="arrow-basis">&#9654;</span> Basis-Effekte
            </div>
            <div class="category-content" id="cat-basis" style="display:none;">
                <button class="mode-btn" data-mode="0">Solid Color</button>
                <button class="mode-btn" data-mode="1">Rainbow</button>
                <button class="mode-btn" data-mode="2">Pulse</button>
                <button class="mode-btn" data-mode="3">Rainbow/Hex</button>
                <button class="mode-btn" data-mode="4">Blink Seq.</button>
                <button class="mode-btn" data-mode="5">Breathe</button>
                <button class="mode-btn" data-mode="6">Strobe</button>
                <button class="mode-btn" data-mode="7">Flicker</button>
                <button class="mode-btn" data-mode="10">Strobe Long</button>
                <button class="mode-btn" data-mode="28">Herzschlag</button>
                <button class="mode-btn" data-mode="29">Funken</button>
            </div>
        </div>

        <div class="mode-category">
            <div class="category-header" onclick="toggleCategory('kreislauf')">
                <span class="arrow" id="arrow-kreislauf">&#9654;</span> Kreislauf-Effekte
            </div>
            <div class="category-content" id="cat-kreislauf" style="display:none;">
                <button class="mode-btn" data-mode="8">Running Light</button>
                <button class="mode-btn" data-mode="9">Chase/Comet</button>
                <button class="mode-btn" data-mode="11">Synchron</button>
                <button class="mode-btn" data-mode="12">Komplementaer</button>
                <button class="mode-btn" data-mode="20">Alternierend</button>
                <button class="mode-btn" data-mode="21">Alt.+Kompl.</button>
            </div>
        </div>

        <div class="mode-category">
            <div class="category-header" onclick="toggleCategory('geometrie')">
                <span class="arrow" id="arrow-geometrie">&#9654;</span> Geometrie-Muster
            </div>
            <div class="category-content" id="cat-geometrie" style="display:none;">
                <button class="mode-btn" data-mode="13">V-Regenbogen</button>
                <button class="mode-btn" data-mode="14">H-Regenbogen</button>
                <button class="mode-btn" data-mode="15">V-Farbverlauf</button>
                <button class="mode-btn" data-mode="35">Farbrad</button>
                <button class="mode-btn" data-mode="16">Welle (Kanten)</button>
                <button class="mode-btn" data-mode="24">Welle (LED)</button>
                <button class="mode-btn" data-mode="34">Wellen-Kollision</button>
                <button class="mode-btn" data-mode="39">Wellenwand</button>
                <button class="mode-btn" data-mode="25">Radial In-Out</button>
                <button class="mode-btn" data-mode="26">Radial Out-In</button>
                <button class="mode-btn" data-mode="36">Atmendes Zentrum</button>
                <button class="mode-btn" data-mode="27">Strudel</button>
                <button class="mode-btn" data-mode="17">Feuer</button>
                <button class="mode-btn" data-mode="18">Aurora</button>
                <button class="mode-btn" data-mode="19">Plasma</button>
                <button class="mode-btn" data-mode="32">Lava Lamp</button>
            </div>
        </div>

        <div class="mode-category">
            <div class="category-header" onclick="toggleCategory('spezial')">
                <span class="arrow" id="arrow-spezial">&#9654;</span> Spezial-Effekte
            </div>
            <div class="category-content" id="cat-spezial" style="display:none;">
                <button class="mode-btn" data-mode="30">Regen</button>
                <button class="mode-btn" data-mode="31">Bouncing Ball</button>
                <button class="mode-btn" data-mode="33">Matrix Rain</button>
                <button class="mode-btn" data-mode="37">Schachbrett</button>
                <button class="mode-btn" data-mode="38">Meteor</button>
            </div>
        </div>

        <div class="mode-category">
            <div class="category-header" onclick="toggleCategory('debug')">
                <span class="arrow" id="arrow-debug">&#9654;</span> Debug
            </div>
            <div class="category-content" id="cat-debug" style="display:none;">
                <button class="mode-btn" data-mode="22">Phys. Kreislauf</button>
                <button class="mode-btn" data-mode="23">Harm. Kreislauf</button>
            </div>
        </div>
    </div>

    <script>
        function togglePower() {
            fetch('/api/power', { method: 'POST' })
                .then(r => r.json())
                .then(d => {
                    document.getElementById('power-status').innerText = d.power ? 'ON' : 'OFF';
                });
        }

        function setColor() {
            const color = document.getElementById('colorPicker').value;
            const r = parseInt(color.substr(1,2), 16);
            const g = parseInt(color.substr(3,2), 16);
            const b = parseInt(color.substr(5,2), 16);

            fetch('/api/color', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ r, g, b })
            });
        }

        function setBrightness() {
            const val = document.getElementById('brightness').value;
            document.getElementById('brightness-value').innerText = val;

            fetch('/api/brightness', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ brightness: parseInt(val) })
            });
        }

        function setSpeed() {
            const val = document.getElementById('speed').value;
            document.getElementById('speed-value').innerText = val;

            fetch('/api/speed', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ speed: parseInt(val) })
            });
        }

        // Mode names for display
        const modeNames = {
            0: 'Solid Color', 1: 'Rainbow', 2: 'Pulse', 3: 'Rainbow/Hex',
            4: 'Blink Seq.', 5: 'Breathe', 6: 'Strobe', 7: 'Flicker',
            8: 'Running Light', 9: 'Chase/Comet', 10: 'Strobe Long',
            11: 'Synchron', 12: 'Komplementaer', 13: 'V-Regenbogen',
            14: 'H-Regenbogen', 15: 'V-Farbverlauf', 16: 'Welle (Kanten)',
            17: 'Feuer', 18: 'Aurora', 19: 'Plasma',
            20: 'Alternierend', 21: 'Alt.+Kompl.',
            22: 'Phys. Kreislauf', 23: 'Harm. Kreislauf',
            24: 'Welle (LED)', 25: 'Radial In-Out', 26: 'Radial Out-In',
            27: 'Strudel', 28: 'Herzschlag', 29: 'Funken',
            30: 'Regen', 31: 'Bouncing Ball', 32: 'Lava Lamp',
            33: 'Matrix Rain', 34: 'Wellen-Kollision', 35: 'Farbrad',
            36: 'Atmendes Zentrum', 37: 'Schachbrett', 38: 'Meteor', 39: 'Wellenwand'
        };

        function toggleCategory(name) {
            const content = document.getElementById('cat-' + name);
            const arrow = document.getElementById('arrow-' + name);
            if (content.style.display === 'none') {
                content.style.display = 'flex';
                arrow.classList.add('open');
            } else {
                content.style.display = 'none';
                arrow.classList.remove('open');
            }
        }

        function setMode(mode) {
            fetch('/api/mode', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ mode: parseInt(mode) })
            }).then(() => updateActiveButton(mode));
        }

        function updateActiveButton(mode) {
            // Remove active from all buttons
            document.querySelectorAll('.mode-btn').forEach(btn => btn.classList.remove('active'));
            // Add active to current
            const activeBtn = document.querySelector('.mode-btn[data-mode="' + mode + '"]');
            if (activeBtn) activeBtn.classList.add('active');
            // Update mode name
            document.getElementById('current-mode-name').innerText = '(' + (modeNames[mode] || 'Mode ' + mode) + ')';
        }

        // Add click handlers to all mode buttons
        document.querySelectorAll('.mode-btn').forEach(btn => {
            btn.addEventListener('click', () => setMode(btn.dataset.mode));
        });

        // Update status periodically
        function updateStatus() {
            fetch('/api/status')
                .then(r => r.json())
                .then(d => {
                    document.getElementById('power-status').innerText = d.power ? 'ON' : 'OFF';
                    document.getElementById('brightness').value = d.brightness;
                    document.getElementById('brightness-value').innerText = d.brightness;
                    document.getElementById('speed').value = d.speed;
                    document.getElementById('speed-value').innerText = d.speed;
                    document.getElementById('geometry-status').innerText =
                        d.geometryLoaded ? ('Geladen (' + d.numHexagons + ' Hex)') : 'NICHT GELADEN';
                    document.getElementById('geometry-status').style.color = d.geometryLoaded ? '#4CAF50' : '#f44336';
                    updateActiveButton(d.mode);

                    // Build-Info anzeigen
                    if (d.buildDate) {
                        const date = new Date(d.buildDate * 1000);
                        const dateStr = date.toLocaleDateString('de-DE') + ' ' + date.toLocaleTimeString('de-DE');
                        document.getElementById('build-info').innerText = dateStr;
                    }
                });
        }
        updateStatus(); // Initial load
        setInterval(updateStatus, 2000);
    </script>
</body>
</html>
)HTMLDELIM";
