// act1/coast.c — hooks de la costa de la isla (escenas 4 y 7 del guión).
// La secuencia está en coast.scene y coast_end.scene; aquí la lógica:
// ambiente, la gaviota que alza el vuelo, la exploración con el árbol rojo
// examinable, el combate físico contra los jabalíes y la curación de Clio.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "narrative/narrative.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/coast.h"

#define COAST_TREE_X       490   // x (mundo) del árbol rojo
#define COAST_COMBAT_SCROLL 200  // offset de scroll donde salta el combate

void act1_coast_arrive(void)    // Llegada: olas y la gaviota alza el vuelo
{
    play_sample(snd_ambient_waves, sizeof(snd_ambient_waves));

    // La gaviota (item 0) alza el vuelo: se oculta el item y un sprite
    // temporal sale volando hacia arriba a la derecha
    play_sample(snd_ambient_seagull, sizeof(snd_ambient_seagull));
    s16 gx = 520 - FASTFIX32_TO_INT(offset_BGA), gy = 128;
    release_item(0);
    Sprite *gull = SPR_addSpriteSafe(&item_coast_seagull, gx, gy,
                                     TILE_ATTR(PAL0, TRUE, false, false));
    for (u16 i = 0; i < SCREEN_FPS * 2 && gull; i++) {
        gx += 2; gy -= 1;
        SPR_setPosition(gull, gx, gy);
        next_frame(false);
        if (gx > SCREEN_WIDTH + 8) break;
    }
    if (gull) { SPR_releaseSprite(gull); SPR_update(); }
}

void act1_coast_explore(void)    // Paseo hasta el combate; el árbol rojo se puede examinar
{
    u16 prev_joy = JOY_readJoypad(JOY_ALL);

    while (FASTFIX32_TO_INT(offset_BGA) < COAST_COMBAT_SCROLL) {
        next_frame(true);

        u16 joy = JOY_readJoypad(JOY_ALL);
        u16 pressed = joy & ~prev_joy;
        prev_joy = joy;

        // Examinar el árbol rojo (pintado en el fondo: zona por posición)
        s16 world_x = FASTFIX32_TO_INT(obj_character[active_character].x) +
                      obj_character[active_character].x_size / 2 +
                      FASTFIX32_TO_INT(offset_BGA);
        if ((pressed & BUTTON_A) && world_x > COAST_TREE_X - 60 && world_x < COAST_TREE_X + 60)
            talk_dialog(&dialogs[ACT1_COAST][A1_COAST_TREE], false);
    }
}

void act1_coast_boars(void)    // Primer combate físico: 3 jabalíes (Linus sin vara)
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

void act1_coast_heal(void)    // Clio canta el patrón de Curación (narrativo)
{
    obj_character[active_character].state = STATE_IDLE;
    anim_character(CHR_linus, ANIM_IDLE);
    look_left(CHR_clio, false);          // mirando a Linus (a su derecha)

    anim_character(CHR_clio, ANIM_MAGIC);
    play_sample(snd_effect_magic_appear, sizeof(snd_effect_magic_appear));

    // Destello verde en el cielo mientras canta
    u16 old = PAL_getColor(1);           // color 1 = cielo de la paleta de costa
    for (u16 i = 0; i < SCREEN_FPS * 2; i++) {
        PAL_setColor(1, ((i >> 3) & 1) ? RGB24_TO_VDPCOLOR(0x55cc77) : old);
        next_frame(false);
    }
    PAL_setColor(1, old);
    anim_character(CHR_clio, ANIM_IDLE);
}

void act1_coast_end_ambient(void)    // Cierre del acto: la nave prepara la partida
{
    play_sample(snd_ambient_waves, sizeof(snd_ambient_waves));
    play_sample(snd_ambient_steam, sizeof(snd_ambient_steam));
}
