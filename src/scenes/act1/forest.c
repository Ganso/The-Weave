// act1/forest.c — hooks del bosque (lógica; la secuencia está en forest.scene)

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/forest.h"

void act1_forest_pad_hint(void)    // Aviso si el jugador usa un mando de 3 botones
{
    dprintf(2,"Detected controller type: %d\n", JOY_getJoypadType(JOY_1));
    if (JOY_getJoypadType(JOY_1) == JOY_TYPE_PAD3) {
        talk_cluster(&dialogs[ACT1_FOREST][A1_FOREST_3BUTTONS], true);
    }
}

void act1_forest_day(void)    // Fade a la paleta de día del bosque
{
    PAL_fadeTo(0, 15, forest_pal.data, SCREEN_FPS, false);
}

void act1_forest_enemies(void)    // Aparición de los dos WeaverGhosts (previo al op combat)
{
    show_or_hide_interface(false);

    PAL_setPalette(PAL_ENEMIES, weaver_ghost_sprite.palette->data, DMA); // Enemy palette

    // Stop everybody before ghosts appear
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    init_enemy(1, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy_instant(1, FASTFIX32_FROM_INT(-20), FASTFIX32_FROM_INT(156));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(136));
    move_enemy(1, FASTFIX32_FROM_INT(20), FASTFIX32_FROM_INT(156));
}
