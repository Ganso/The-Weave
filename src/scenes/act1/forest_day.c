// act1/forest_day.c — hooks del bosque de día de la isla (escena 4b del
// guión: tras la playa, el interior; aquí atacan por primera vez los
// jabalíes y Clio canta Curación). La secuencia está en forest_day.scene.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/forest_day.h"

void act1_fday_boars(void)    // Primer combate físico: 3 jabalíes (Linus sin vara)
{
    PAL_setPalette(PAL3, boar_sprite.palette->data, DMA);
    static const s16 spawn[3][2] = {    // {x esquina, y de pies}
        { SCREEN_WIDTH + 12, 146 },
        { SCREEN_WIDTH + 40, 170 },
        { -60,               158 },
    };
    for (u16 i = 0; i < 3; i++) {
        init_enemy(i, ENEMY_CLS_BOAR);
        move_enemy_instant(i, FASTFIX32_FROM_INT(spawn[i][0]), FASTFIX32_FROM_INT(spawn[i][1]));
        show_enemy(i, true);
    }
    player_max_hitpoints = 5;
    melee_combat_run(4, CHR_clio);   // 4 golpes los ahuyentan
}

void act1_fday_heal(void)    // Clio canta el patrón de Curación (narrativo)
{
    obj_character[active_character].state = STATE_IDLE;
    anim_character(CHR_linus, ANIM_IDLE);
    look_left(CHR_clio, false);          // mirando a Linus (a su derecha)

    anim_character(CHR_clio, ANIM_MAGIC);
    play_sample(snd_effect_magic_appear, sizeof(snd_effect_magic_appear));

    // Destello verde en el cielo del bosque mientras canta
    u16 old = PAL_getColor(4);           // color 4 = azul del cielo en forest_pal
    for (u16 i = 0; i < SCREEN_FPS * 2; i++) {
        PAL_setColor(4, ((i >> 3) & 1) ? RGB24_TO_VDPCOLOR(0x55cc77) : old);
        next_frame(false);
    }
    PAL_setColor(4, old);
    anim_character(CHR_clio, ANIM_IDLE);
}
