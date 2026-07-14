// scene_hooks.c — tabla de despacho de hooks. Los hooks viven en src/scenes/<acto>/<escena>.c

#include <genesis.h>
#include "scenes/scenes.h"
#include "scenes/act1/bedroom.h"
#include "scenes/act1/corridor.h"
#include "scenes/act1/forest.h"
#include "scenes/act1/test.h"
#include "scenes/act1/coast.h"
#include "scenes/act1/hut.h"
#include "scenes/act1/ret.h"

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
    [HOOK_ACT1_COAST_ARRIVE]     = act1_coast_arrive,
    [HOOK_ACT1_COAST_BOARS]      = act1_coast_boars,
    [HOOK_ACT1_COAST_HEAL]       = act1_coast_heal,
    [HOOK_ACT1_COAST_END_AMBIENT] = act1_coast_end_ambient,
    [HOOK_ACT1_HUT_LIGHTNING]    = act1_hut_lightning,
    [HOOK_ACT1_RETURN_START]     = act1_return_start,
    [HOOK_ACT1_RETURN_BOARS]     = act1_return_boars,
    [HOOK_ACT1_RETURN_GHOSTS]    = act1_return_ghosts,
    [HOOK_ACT1_RETURN_END]       = act1_return_end,
};
