// scene_hooks.c — tabla de despacho de hooks. Los hooks viven en src/scenes/<acto>/<escena>.c

#include <genesis.h>
#include "scenes/scenes.h"
#include "scenes/act1/bedroom.h"
#include "scenes/act1/corridor.h"
#include "scenes/act1/forest.h"
#include "scenes/act1/test.h"

const SceneHook scene_hook_table[HOOK_COUNT] = {
    [HOOK_ACT1_BEDROOM_SWAN]     = act1_bedroom_swan,
    [HOOK_ACT1_BEDROOM_WAKE]     = act1_bedroom_wake,
    [HOOK_ACT1_BEDROOM_ITEMS]    = act1_bedroom_items,
    [HOOK_ACT1_CORRIDOR_ITEMS]   = act1_corridor_items,
    [HOOK_ACT1_FOREST_PAD_HINT]  = act1_forest_pad_hint,
    [HOOK_ACT1_FOREST_DAY]       = act1_forest_day,
    [HOOK_ACT1_FOREST_ENEMIES]   = act1_forest_enemies,
    [HOOK_ACT1_TEST_GHOST]       = act1_test_ghost,
    [HOOK_ACT1_TEST_GHOST2]      = act1_test_ghost2,
    [HOOK_ACT1_TEST_BOARS]       = act1_test_boars,
};
