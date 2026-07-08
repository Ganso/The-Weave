// act1/test.c — hooks de la escena de PRUEBA del motor (la secuencia está en test.scene).
// Reutiliza recursos existentes: fondo de bosque + el WeaverGhost. Nada nuevo en res/.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/test.h"

void act1_test_ghost2(void)    // Oleada 2: fantasma de TEST con dos hechizos (thunder + mordisco)
{
    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);

    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_TESTGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
}

void act1_test_ghost(void)    // Un WeaverGhost para el combate de prueba (patrón de forest)
{
    show_or_hide_interface(false);

    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA); // Enemy palette

    // Parar a todo el mundo antes de que aparezca
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(140));
}
