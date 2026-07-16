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

void act1_coast_arrive(void)    // Llegada: el rumor de las olas
{
    play_sample(snd_ambient_waves, sizeof(snd_ambient_waves));
}

// La gaviota (item 0) alza el vuelo: se oculta el item y un sprite temporal
// sale volando hacia arriba a la derecha
static void coast_gull_flies(void)
{
    play_sample(snd_ambient_seagull, sizeof(snd_ambient_seagull));
    s16 gx = 520 - FASTFIX32_TO_INT(offset_BGA), gy = 112;
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

void act1_coast_explore(void)    // Paseo: la gaviota vuela al llegar al árbol, que se puede examinar
{
    u16 prev_joy = JOY_readJoypad(JOY_ALL);
    bool gull_flew = false;

    while (FASTFIX32_TO_INT(offset_BGA) < COAST_COMBAT_SCROLL) {
        next_frame(true);

        s16 world_x = FASTFIX32_TO_INT(obj_character[active_character].x) +
                      obj_character[active_character].x_size / 2 +
                      FASTFIX32_TO_INT(offset_BGA);

        // Al alcanzar la altura del árbol, la gaviota alza el vuelo (guión 4.1)
        if (!gull_flew && world_x > COAST_TREE_X - 130) {
            gull_flew = true;
            coast_gull_flies();
        }

        u16 joy = JOY_readJoypad(JOY_ALL);
        u16 pressed = joy & ~prev_joy;
        prev_joy = joy;

        // Examinar el árbol rojo (pintado en el fondo: zona por posición)
        if ((pressed & BUTTON_A) && world_x > COAST_TREE_X - 60 && world_x < COAST_TREE_X + 60)
            talk_dialog(&dialogs[ACT1_COAST][A1_COAST_TREE], false);
    }
}

void act1_coast_end_ambient(void)    // Cierre del acto: la nave prepara la partida
{
    play_sample(snd_ambient_waves, sizeof(snd_ambient_waves));
    play_sample(snd_ambient_steam, sizeof(snd_ambient_steam));
}

// Fundido a negro con ambos embarcando; la voz de Bobbin suena en la
// oscuridad (se restauran solo las paletas del texto, el interfaz de diálogo
// y la cara del cisne) y el FIN DEL ACTO cierra la pantalla.
void act1_end_epilogue(void)
{
    PAL_fadeOutAll(SCREEN_FPS * 2, false);
    show_character(CHR_linus, false);
    show_character(CHR_clio, false);
    PAL_setPalette(PAL1, characters_pal.data, DMA);   // cara del cisne
    PAL_setPalette(PAL2, interface_pal.data, DMA);    // caja de diálogo
    PAL_setColor(15, RGB24_TO_VDPCOLOR(0xEEEEEE));    // texto
    talk_cluster(&dialogs[ACT1_COAST_END][A1_END_BOBBIN], true);
    talk_dialog(&dialogs[ACT1_COAST_END][A1_END_FIN], true);
}
