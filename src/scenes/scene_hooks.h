#ifndef _SCENE_HOOKS_H_
#define _SCENE_HOOKS_H_

// Hooks C de las escenas: la LÓGICA que el DSL no debe expresar (setup con
// punteros a recursos, bucles de interacción con items, cinemáticas de paleta).
// El DSL los invoca con `call <nombre>` (op SCENE_OP_CALL); gen_scenes.py valida
// los nombres contra el enum HOOK_* de este archivo (nombre DSL = minúsculas).
// Un hook puede bloquear usando next_frame(), igual que el código de escena de siempre.

#include <genesis.h>

typedef void (*SceneHook)(void);

// IMPORTANTE: gen_scenes.py parsea este enum. Formato: HOOK_<NOMBRE>, uno por línea.
typedef enum {
    HOOK_ACT1_SCENE1_SETUP = 0,
    HOOK_ACT1_SCENE1_SWAN,
    HOOK_ACT1_SCENE1_WAKE,
    HOOK_ACT1_SCENE1_ITEMS,
    HOOK_ACT1_SCENE2_SETUP,
    HOOK_ACT1_SCENE2_ENTRY,
    HOOK_ACT1_SCENE2_ITEMS,
    HOOK_ACT1_SCENE3_SETUP,
    HOOK_ACT1_SCENE5_SETUP,
    HOOK_ACT1_SCENE5_PAD_HINT,
    HOOK_ACT1_SCENE5_DAY,
    HOOK_ACT1_SCENE5_ENEMIES,
    HOOK_COUNT
} SceneHookId;

extern const SceneHook scene_hook_table[HOOK_COUNT]; // tabla de despacho (indexada por SceneHookId)

#endif
