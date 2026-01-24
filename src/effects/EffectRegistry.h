#pragma once

#include "Effect.h"

#define MAX_EFFECTS 50

/**
 * EffectRegistry - Verwaltet alle registrierten Effekte
 *
 * Singleton-Pattern für globalen Zugriff
 */
class EffectRegistry {
public:
    static EffectRegistry& getInstance() {
        static EffectRegistry instance;
        return instance;
    }

    /**
     * Registriert einen Effekt unter einer ID
     * @param id Effekt-ID (0-49)
     * @param effect Pointer auf Effekt-Instanz (wird nicht freigegeben)
     */
    void registerEffect(uint8_t id, Effect* effect) {
        if (id < MAX_EFFECTS) {
            effects[id] = effect;
            if (id >= effectCount) {
                effectCount = id + 1;
            }
        }
    }

    /**
     * Gibt den Effekt für eine ID zurück
     * @return Effect-Pointer oder nullptr wenn nicht registriert
     */
    Effect* getEffect(uint8_t id) {
        if (id < MAX_EFFECTS) {
            return effects[id];
        }
        return nullptr;
    }

    /**
     * Führt den Effekt mit der gegebenen ID aus
     */
    void runEffect(uint8_t id, EffectContext& ctx) {
        Effect* effect = getEffect(id);
        if (effect) {
            effect->update(ctx);
        }
    }

    /**
     * Wechselt zum neuen Effekt und ruft reset() auf
     */
    void switchTo(uint8_t id) {
        if (currentEffectId != id) {
            Effect* newEffect = getEffect(id);
            if (newEffect) {
                newEffect->reset();
                currentEffectId = id;
            }
        }
    }

    /**
     * Gibt den Namen des Effekts zurück
     */
    const char* getEffectName(uint8_t id) {
        Effect* effect = getEffect(id);
        if (effect) {
            return effect->getName();
        }
        return "Unknown";
    }

    uint8_t getEffectCount() const { return effectCount; }
    uint8_t getCurrentEffectId() const { return currentEffectId; }

private:
    EffectRegistry() : effectCount(0), currentEffectId(0) {
        for (int i = 0; i < MAX_EFFECTS; i++) {
            effects[i] = nullptr;
        }
    }

    Effect* effects[MAX_EFFECTS];
    uint8_t effectCount;
    uint8_t currentEffectId;
};

// Makro für einfache Effekt-Registrierung
#define REGISTER_EFFECT(id, effectClass) \
    static effectClass _effect_##id; \
    static bool _registered_##id = []() { \
        EffectRegistry::getInstance().registerEffect(id, &_effect_##id); \
        return true; \
    }()
