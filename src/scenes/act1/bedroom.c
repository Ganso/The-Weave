// act1/bedroom.c — hooks del dormitorio (lógica; la secuencia está en bedroom.scene)

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "scenes/act1/bedroom.h"
#include "narrative/texts.h"
#include "narrative/texts_data.h"
#include "narrative/dialogs.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/items.h"
#include "world/background.h"
#include "spells/spell.h"
#include "interface/interface.h"
#include "audio/sound.h"
#include "res_backgrounds.h"
#include "res_characters.h"
#include "res_items.h"
#include "res_sound.h"
#include "res_geesebumps.h"
#include "resources.h"

void act1_bedroom_setup(void)    // Nivel nocturno, personajes e items del dormitorio
{
    new_level(&bedroom_bg_tile, &bedroom_bg_map, &bedroom_front_tile, &bedroom_front_map, bedroom_night_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 3);
    set_limits(20,145,278,176);

    // Use swan palette instead of characters palette
    PAL_setPalette(PAL1, swan_pal.data, DMA);

    init_character(CHR_swan);
    init_character(CHR_linus);

    init_item(0, &item_bedroom_bed, PAL0, 31, 139, 93, 0, 23, 0, FORCE_BACKGROUND); // Bed
    init_item(1, &item_bedroom_chair, PAL0, 236, 110, 26, 0, 43, 0, FORCE_BACKGROUND); // Chair
    init_item(2, &item_bedroom_windowsill, PAL0, 125, 121, 99, 0, 22, 0, FORCE_BACKGROUND); // Windowsill
    init_item(3, &item_bedroom_cabinet, PAL0, 270, 79, 51, 0, 82, 0, FORCE_BACKGROUND); // Cabinet and sheet music
    init_item(4, &item_bedroom_linus_sleeping, PAL0, 30, 112, 0, 0, 0, 0, FORCE_BACKGROUND); // Linus sleeping
}

void act1_bedroom_swan(void)    // Cinemática del cisne: aparece con flash, habla, desaparece
{
    u16 paltmp[64];

    // Flash to white, show swan, fade back
    wait_seconds(2);
    PAL_getColors(0, paltmp, 64); // backup current palette
    play_sample(snd_effect_magic_appear, sizeof(snd_effect_magic_appear));
    PAL_fadeToAll(geesebumps_pal_white.data, SCREEN_FPS, false);
    move_character_instant(CHR_swan, 141, 110);
    show_character(CHR_swan, true);
    PAL_fadeToAll(paltmp, SCREEN_FPS, false);
    wait_seconds(2);

    talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_HUNDRED_YEARS], true); // (ES) "Han pasado ya cien años..." - (EN) "A hundred years have passed..."

    // Flash to white, hide swan, fade back
    wait_seconds(2);
    PAL_getColors(0, paltmp, 64);
    play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear));
    PAL_fadeToAll(geesebumps_pal_white.data, SCREEN_FPS, false);
    show_character(CHR_swan, false);
    PAL_fadeToAll(paltmp, SCREEN_FPS, false);
    wait_seconds(2);
}

void act1_bedroom_wake(void)    // Amanece: paleta de día, Linus se levanta, control al jugador
{
    PAL_fadeTo(0, 15, bedroom_pal.data, SCREEN_FPS, false); // daytime

    talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_NEXT_MORNING], true); // (ES) "A la mañana siguiente..." - (EN) "The next morning..."

    release_item(4); // Linus sleeping
    PAL_setPalette(PAL1, characters_pal.data, DMA);
    move_character_instant(CHR_linus, 35, 175);
    show_character(CHR_linus, true);

    player_scroll_active = true;
    movement_active = true;
}

void act1_bedroom_items(void)    // Bucle de items del dormitorio (lógica NO expresable en DSL lineal)
{
    bool item_interacted[4] = {false,false,false,false};
    u16 scene_timeout = 0; // Frame counter (B23)

    while (scene_timeout < (SCREEN_FPS*3)) // La escena acaba 3 s después del cabinet + pausa
    {
        switch (last_interacted_item)
        {
        case 0: // Bed
            talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_SLEPT_BAD], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[0] = true;
            scene_timeout = 0;
            break;
        case 1: // Chair
            talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_NO_TIME_TO_SIT], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[1] = true;
            scene_timeout = 0;
            break;
        case 2: // Windowsill
            talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_NOT_REAL], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[2] = true;
            scene_timeout = 0;
            break;
        case 3: // Cabinet
            talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_LULLABY], false);
            last_interacted_item = ITEM_NONE;
            if (item_interacted[3] == false) { // Solo la primera vez
                activate_spell(SPELL_SLEEP);
                talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_LEARNED_PATTERN], true);
            }
            item_interacted[3] = true;
            break;
        default:
            break;
        }
        // El tiempo solo corre tras el cabinet Y haber abierto la pausa
        if (item_interacted[3] == true && player_has_paused) scene_timeout++;
        next_frame(true);
    }
}
