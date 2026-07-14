// act1/hut.c — hooks de la choza de los Tejedores (escena 5 del guión).
// La secuencia está en hut.scene; aquí el momento clave: el rayo cae sobre
// el bastón, Linus lo recoge (anim GRAB) y se inscribe Electricidad.

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "combat/combat.h"
#include "spells/spells.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/hut.h"

void act1_hut_lightning(void)    // Rayo + recogida del bastón + patrón Electricidad
{
    // Parar a todo el mundo
    obj_character[active_character].state = STATE_IDLE;
    anim_character(CHR_linus, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    // El rayo cae por el agujero del techo sobre el bastón (item 0, ~x110 de pantalla)
    PAL_setPalette(PAL3, fx_lightning_sprite.palette->data, DMA);
    Sprite *bolt = SPR_addSpriteSafe(&fx_lightning_sprite, 100, 8,
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
    player_has_rod = true;
    reinit_character_sprite(CHR_linus);

    activate_spell(SPELL_THUNDER);   // patrón inscrito: Electricidad (jingle + notas)
}
