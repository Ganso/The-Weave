#ifndef _SCENE_HOOKS_H_
#define _SCENE_HOOKS_H_

// Registro central de hooks de escena. Los hooks viven en src/scenes/<acto>/<escena>.c
// (un par .c/.h por escena); aquí solo el enum y la tabla de despacho.
// El DSL los invoca con `call <nombre>` (op SCENE_OP_CALL); gen_scenes.py valida
// los nombres contra el enum HOOK_* (nombre DSL = minúsculas, sin el prefijo HOOK_).
// Un hook puede bloquear usando next_frame(), igual que el código de escena de siempre.

#include <genesis.h>

typedef void (*SceneHook)(void);

// IMPORTANTE: gen_scenes.py parsea este enum. Formato: HOOK_<NOMBRE>, uno por línea.
typedef enum {
    HOOK_ACT1_BEDROOM_SETUP = 0,
    HOOK_ACT1_BEDROOM_SWAN,
    HOOK_ACT1_BEDROOM_WAKE,
    HOOK_ACT1_BEDROOM_ITEMS,
    HOOK_ACT1_CORRIDOR_SETUP,
    HOOK_ACT1_CORRIDOR_ENTRY,
    HOOK_ACT1_CORRIDOR_ITEMS,
    HOOK_ACT1_HALL_SETUP,
    HOOK_ACT1_FOREST_SETUP,
    HOOK_ACT1_FOREST_PAD_HINT,
    HOOK_ACT1_FOREST_DAY,
    HOOK_ACT1_FOREST_ENEMIES,
    HOOK_ACT1_TEST_SETUP,
    HOOK_ACT1_TEST_GHOST,
    HOOK_ACT1_TEST_GHOST2,
    HOOK_COUNT
} SceneHookId;

extern const SceneHook scene_hook_table[HOOK_COUNT]; // tabla de despacho (indexada por SceneHookId)

#endif
