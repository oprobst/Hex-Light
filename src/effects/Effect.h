#pragma once

#include "EffectContext.h"

/**
 * Effect - Basisklasse für alle LED-Effekte
 *
 * Jeder Effekt muss update() implementieren, das einmal pro Frame aufgerufen wird.
 * Der Effekt setzt die LEDs im übergebenen Context.
 */
class Effect {
public:
    virtual ~Effect() = default;

    /**
     * Aktualisiert den Effekt und setzt die LEDs
     * Wird ca. 50x pro Sekunde aufgerufen
     */
    virtual void update(EffectContext& ctx) = 0;

    /**
     * Name des Effekts für UI-Anzeige
     */
    virtual const char* getName() const = 0;

    /**
     * Wird aufgerufen wenn der Effekt aktiviert wird
     * Kann überschrieben werden um State zurückzusetzen
     */
    virtual void reset() {}
};
