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
    // Linus pierde la vara y saca la antorcha (cambia también su sprite)
    player_has_rod = false;
    linus_has_torch = true;
    reinit_character_sprite(CHR_linus);

    // 5 jabalíes fuera de pantalla: tres por la derecha y dos por la izquierda
    PAL_setPalette(PAL3, boar_sprite.palette->data, DMA);
    static const s16 boar_spawn[5][2] = {   // {x esquina, y de pies}
        { SCREEN_WIDTH + 12, 146 },         // derecha, arriba
        { -60,               152 },         // izquierda
        { SCREEN_WIDTH + 36, 170 },         // derecha, abajo
        { -84,               166 },         // izquierda
        { SCREEN_WIDTH + 60, 158 },         // derecha, centro
    };
    for (u16 i = 0; i < 5; i++) {
        init_enemy(i, ENEMY_CLS_BOAR);
        move_enemy_instant(i, FASTFIX32_FROM_INT(boar_spawn[i][0]),
                           FASTFIX32_FROM_INT(boar_spawn[i][1]));
        show_enemy(i, true);
    }

    player_max_hitpoints = 5;        // vida del jugador en este combate
    melee_combat_run(&(MeleeConfig){
        .hits_to_win = 6,            // 6 golpes los ahuyentan
        .companion = CHR_clio,       // Clio espera detrás
        .reposition_companion = true,
        .weapon_is_thunder = false,  // el arma es el golpe con A
    });

    // Restaurar la vara: el resto del banco de pruebas la necesita
    linus_has_torch = false;
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

    player_max_hitpoints = 5;   // vida del jugador en este combate

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
