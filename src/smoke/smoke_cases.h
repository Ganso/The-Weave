// smoke_cases.h — tabla de casos de la smoke ROM.
// Añadir un caso = una fila aquí + rebuild (sin tocar menú ni runner).
// Todo src/smoke/ solo se compila con -DHACK_SMOKE_BUILD (build-theweave.sh smoke).
#ifndef _SMOKE_CASES_H_
#define _SMOKE_CASES_H_

#include <genesis.h>
#include "spells/constants_spells.h"

typedef enum {
    SMOKE_CHECK,   // invariante de canUse: PASS si spell_can_use == expected
    SMOKE_CAST,    // cast scripted en un nivel de pruebas: PASS si termina en baseDuration±2
    SMOKE_SCENE    // ejecuta la escena completa (interactiva)
} SmokeKind;

typedef struct {
    const char *name;      // etiqueta del menú
    u8   kind;             // SmokeKind
    u8   spellId;          // CHECK/CAST
    bool reversed;         // CHECK
    u8   zone;             // CHECK: spell_zone durante la comprobación
    bool expected;         // CHECK: resultado esperado de canUse
    const char *sceneName; // SCENE: nombre de escena (scene_id_by_name)
} SmokeCase;

static const SmokeCase smoke_cases[] = {
    // --- Invariantes de canUse (instantáneos) ---
    {"CHK fire sin zona  -> NO",  SMOKE_CHECK, SPELL_FIRE,    false, ZONE_NONE,     false, NULL},
    {"CHK fire caldero   -> SI",  SMOKE_CHECK, SPELL_FIRE,    false, ZONE_CAULDRON, true,  NULL},
    {"CHK thunder inv    -> NO",  SMOKE_CHECK, SPELL_THUNDER, true,  ZONE_NONE,     false, NULL}, // sin ventana de counter
    {"CHK hide directo   -> SI",  SMOKE_CHECK, SPELL_HIDE,    false, ZONE_NONE,     true,  NULL},
    {"CHK open directo   -> NO",  SMOKE_CHECK, SPELL_OPEN,    false, ZONE_NONE,     false, NULL}, // solo scripted

    // --- Casts scripted (duración medida + efecto visual a ojo) ---
    {"CAST thunder (4s flash)",   SMOKE_CAST,  SPELL_THUNDER, false, 0, false, NULL},
    {"CAST hide (4s parpadeo)",   SMOKE_CAST,  SPELL_HIDE,    false, 0, false, NULL},
    {"CAST open (0.75s)",         SMOKE_CAST,  SPELL_OPEN,    false, 0, false, NULL},
    {"CAST sleep (1.25s)",        SMOKE_CAST,  SPELL_SLEEP,   false, 0, false, NULL},
    {"CAST fire (2s naranja)",    SMOKE_CAST,  SPELL_FIRE,    false, 0, false, NULL},

    // --- Escenas completas (interactivas; forest acaba en reset: es su final) ---
    {"SCENE act1_bedroom",        SMOKE_SCENE, 0, false, 0, false, "act1_bedroom"},
    {"SCENE act1_corridor",       SMOKE_SCENE, 0, false, 0, false, "act1_corridor"},
    {"SCENE act1_hall",           SMOKE_SCENE, 0, false, 0, false, "act1_hall"},
    {"SCENE act1_forest",         SMOKE_SCENE, 0, false, 0, false, "act1_forest"},
};

#define SMOKE_CASE_COUNT (sizeof(smoke_cases)/sizeof(smoke_cases[0]))

#endif
