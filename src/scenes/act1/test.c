// act1/test.c — hooks de la escena de PRUEBA del motor (la secuencia está en test.scene).
// Reutiliza recursos existentes: fondo de bosque + el WeaverGhost. Nada nuevo en res/.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/test.h"

void act1_test_boars(void)    // Combate físico: Linus sin vara contra 3 jabalíes (melee.c)
{
    // Linus pierde la vara para el combate (cambia también su sprite)
    player_has_rod = false;
    reinit_character_sprite(CHR_linus);

    // 3 jabalíes por la derecha a tres alturas, fuera de pantalla
    PAL_setPalette(PAL3, boar_sprite.palette->data, DMA);
    static const s16 boar_feet_y[3] = {146, 158, 170};
    for (u16 i = 0; i < 3; i++) {
        init_enemy(i, ENEMY_CLS_BOAR);
        move_enemy_instant(i, FASTFIX32_FROM_INT(SCREEN_WIDTH + 12 + i * 24),
                           FASTFIX32_FROM_INT(boar_feet_y[i]));
        show_enemy(i, true);
    }

    melee_combat_run(3, CHR_clio);   // 3 golpes los ahuyentan; Clio espera detrás

    // Restaurar la vara: el resto del banco de pruebas la necesita
    player_has_rod = true;
    reinit_character_sprite(CHR_linus);
}

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
