// act1/bedroom.c — hooks del dormitorio (lógica; la secuencia está en bedroom.scene)

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "audio/audio.h"
#include "res_all.h"
#include "scenes/act1/bedroom.h"

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
    talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_AWAKE], false); // (ES) "Otra vez|El mismo sueño..."
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
        case 5: // Retrato familiar
            talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_PORTRAIT], false);
            last_interacted_item = ITEM_NONE;
            scene_timeout = 0;
            break;
        case 7: // Baúl de infancia
            talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_CHEST], false);
            last_interacted_item = ITEM_NONE;
            scene_timeout = 0;
            break;
        case 3: // Cabinet (armario / partitura)
            if (item_interacted[3] == false) { // Primera vez: la nana + el patrón
                talk_dialog(&dialogs[ACT1_BEDROOM][A1_BEDROOM_LULLABY], false);
                activate_spell(SPELL_SLEEP);
                talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_LEARNED_PATTERN], true);
            } else { // Relecturas: "Cuatro notas..."
                talk_cluster(&dialogs[ACT1_BEDROOM][A1_BEDROOM_FOURNOTES], false);
            }
            last_interacted_item = ITEM_NONE;
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
