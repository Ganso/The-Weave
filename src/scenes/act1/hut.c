// act1/hut.c — hooks de la choza de los Tejedores (escena 5 del guión).
// La secuencia está en hut.scene; aquí la lógica: la exploración con los
// puntos de inspección, el rayo sobre el bastón (con recogida y patrón
// Electricidad) y la sombra que se mueve entre los restos al salir.

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
#include "scenes/act1/hut.h"

#define HUT_STAFF_X   430   // x (mundo) del bastón (item 0)
#define HUT_LOOM_X    350   // x (mundo) del centro del telar roto

void act1_hut_start(void)    // Dentro de la choza no se lleva la antorcha (sprite norod)
{
    linus_has_torch = false;
}

// Exploración libre: bastidor (item 1) e hilos (item 2) son inspecciones
// opcionales; interactuar con el bastón (item 0) dispara la cutscene y sale.
void act1_hut_items(void)
{
    while (true) {
        switch (last_interacted_item)
        {
        case 0: // El bastón: arranca la cutscene del rayo
            last_interacted_item = ITEM_NONE;
            return;
        case 1: // Bastidor roto ("Mira esos marcos...")
            talk_cluster(&dialogs[ACT1_HUT][A1_HUT_DEBRIS], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 2: // Hilos ennegrecidos
            talk_dialog(&dialogs[ACT1_HUT][A1_HUT_THREADS], false);
            last_interacted_item = ITEM_NONE;
            break;
        default:
            break;
        }
        next_frame(true);
    }
}

void act1_hut_lightning(void)    // Rayo + recogida del bastón + patrón Electricidad
{
    // Parar a todo el mundo
    obj_character[active_character].state = STATE_IDLE;
    anim_character(CHR_linus, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    // El rayo cae por el agujero del techo sobre el bastón (posición según scroll)
    s16 bolt_x = HUT_STAFF_X - FASTFIX32_TO_INT(offset_BGA) - 12;
    PAL_setPalette(PAL3, fx_lightning_sprite.palette->data, DMA);
    Sprite *bolt = SPR_addSpriteSafe(&fx_lightning_sprite, bolt_x, 8,
                                     TILE_ATTR(PAL3, TRUE, false, false));
    play_sample(snd_ambient_thunder, sizeof(snd_ambient_thunder));

    u16 old1 = PAL_getColor(1), old2 = PAL_getColor(2);   // cielo nocturno (capa B)
    for (u16 i = 0; i < SCREEN_FPS; i++) {
        bool on = (i >> 2) & 1;
        if (bolt) SPR_setFrame(bolt, on ? 1 : 0);
        PAL_setColor(1, on ? RGB24_TO_VDPCOLOR(0xEEEEEE) : old1);
        PAL_setColor(2, on ? RGB24_TO_VDPCOLOR(0xEEEEEE) : old2);
        next_frame(false);
    }
    PAL_setColor(1, old1);
    PAL_setColor(2, old2);
    if (bolt) {
        SPR_setFrame(bolt, 2);                             // desvanecimiento
        for (u16 i = 0; i < 12; i++) next_frame(false);
        SPR_releaseSprite(bolt);
        SPR_update();
    }

    // Linus recoge el bastón: animación de agacharse, el item desaparece y
    // cambia al sprite con vara
    anim_character(CHR_linus, ANIM_GRAB);
    wait_seconds(1);
    release_item(0);
    linus_has_torch = false;   // la antorcha se apaga: ahora luce el bastón
    player_has_rod = true;
    reinit_character_sprite(CHR_linus);

    activate_spell(SPELL_THUNDER);   // patrón inscrito: Electricidad (jingle + notas)

    // De momento solo puede tocar las 3 primeras notas: el resto se irán
    // habilitando más adelante (una nota superior avisa y cancela el patrón)
    player_note_limit = NOTE_SOL;
}

// Al salir, una sombra se mueve entre los restos del telar (placeholder:
// la silueta del espectro cruza el fondo un instante)
void act1_hut_shadow(void)
{
    s16 sx = HUT_LOOM_X - FASTFIX32_TO_INT(offset_BGA);
    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA);
    Sprite *shade = SPR_addSpriteSafe(&weaver_ghost_sprite, sx, 40,
                                      TILE_ATTR(PAL3, FALSE, false, false));
    play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear));
    for (u16 i = 0; i < SCREEN_FPS && shade; i++) {
        SPR_setPosition(shade, sx - i, 40 + (i >> 2));
        SPR_setVisibility(shade, (i & 2) ? VISIBLE : HIDDEN);   // parpadeo espectral
        next_frame(false);
    }
    if (shade) { SPR_releaseSprite(shade); SPR_update(); }
}
