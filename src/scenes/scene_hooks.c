// scene_hooks.c — lógica C de las escenas del acto 1 (trasplantada de act_1.c).
// Cada hook es pequeño y hace UNA cosa; la secuencia narrativa vive en data/scenes/*.scene.

#include <genesis.h>
#include "core/config.h"
#include "core/frame.h"
#include "core/init.h"
#include "core/controller.h"
#include "scenes/scene_hooks.h"
#include "narrative/texts.h"
#include "narrative/texts_data.h"
#include "narrative/dialogs.h"
#include "actors/entity.h"
#include "actors/characters.h"
#include "actors/enemies.h"
#include "actors/items.h"
#include "world/background.h"
#include "combat/combat.h"
#include "spells/spell.h"
#include "spells/notes.h"
#include "interface/interface.h"
#include "audio/sound.h"
#include "res_backgrounds.h"
#include "res_characters.h"
#include "res_items.h"
#include "res_enemies.h"
#include "res_sound.h"
#include "res_geesebumps.h"
#include "resources.h"

// =====================================================================
// ESCENA 1 — Dormitorio
// =====================================================================

static void act1_scene1_setup(void)    // Nivel nocturno, personajes e items del dormitorio
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

static void act1_scene1_swan(void)    // Cinemática del cisne: aparece con flash, habla, desaparece
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

    talk_cluster(&dialogs[ACT1_DIALOG4][A1D4_HUNDRED_YEARS], true); // (ES) "Han pasado ya cien años..." - (EN) "A hundred years have passed..."

    // Flash to white, hide swan, fade back
    wait_seconds(2);
    PAL_getColors(0, paltmp, 64);
    play_sample(snd_effect_magic_disappear, sizeof(snd_effect_magic_disappear));
    PAL_fadeToAll(geesebumps_pal_white.data, SCREEN_FPS, false);
    show_character(CHR_swan, false);
    PAL_fadeToAll(paltmp, SCREEN_FPS, false);
    wait_seconds(2);
}

static void act1_scene1_wake(void)    // Amanece: paleta de día, Linus se levanta, control al jugador
{
    PAL_fadeTo(0, 15, bedroom_pal.data, SCREEN_FPS, false); // daytime

    talk_dialog(&dialogs[ACT1_DIALOG4][A1D4_NEXT_MORNING], true); // (ES) "A la mañana siguiente..." - (EN) "The next morning..."

    release_item(4); // Linus sleeping
    PAL_setPalette(PAL1, characters_pal.data, DMA);
    move_character_instant(CHR_linus, 35, 175);
    show_character(CHR_linus, true);

    player_scroll_active = true;
    movement_active = true;
}

static void act1_scene1_items(void)    // Bucle de items del dormitorio (lógica NO expresable en DSL lineal)
{
    bool item_interacted[4] = {false,false,false,false};
    u16 scene_timeout = 0; // Frame counter (B23)

    while (scene_timeout < (SCREEN_FPS*3)) // La escena acaba 3 s después del cabinet + pausa
    {
        switch (last_interacted_item)
        {
        case 0: // Bed
            talk_dialog(&dialogs[ACT1_DIALOG4][A1D4_SLEPT_BAD], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[0] = true;
            scene_timeout = 0;
            break;
        case 1: // Chair
            talk_dialog(&dialogs[ACT1_DIALOG4][A1D4_NO_TIME_TO_SIT], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[1] = true;
            scene_timeout = 0;
            break;
        case 2: // Windowsill
            talk_dialog(&dialogs[ACT1_DIALOG4][A1D4_NOT_REAL], false);
            last_interacted_item = ITEM_NONE;
            item_interacted[2] = true;
            scene_timeout = 0;
            break;
        case 3: // Cabinet
            talk_dialog(&dialogs[ACT1_DIALOG4][A1D4_LULLABY], false);
            last_interacted_item = ITEM_NONE;
            if (item_interacted[3] == false) { // Solo la primera vez
                activate_spell(SPELL_SLEEP);
                talk_cluster(&dialogs[ACT1_DIALOG4][A1D4_LEARNED_PATTERN], true);
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

// =====================================================================
// ESCENA 2 — Pasillo de los historiadores
// =====================================================================

static void act1_scene2_setup(void)    // Nivel del pasillo + libros/lámparas/puertas/mapas
{
    new_level(NULL, NULL, &historians_corridor_front_tile, &historians_corridor_front_map, historians_corridor_pal, 800, BG_SCRL_USER_LEFT, 0);
    set_limits(0,140,275,168);

    init_item(0, &item_corridor_bookpedestal_sprite, PAL0, 600, 95, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, CALCULATE_DEPTH); // Guild history book
    init_item(1, &item_corridor_bookpedestal_sprite, PAL0, 200, 95, COLLISION_DEFAULT, COLLISION_DEFAULT, 8, 58, CALCULATE_DEPTH); // Myths and legends
    init_item(2, &item_corridor_lamp_sprite, PAL0, 32, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(3, &item_corridor_lamp_sprite, PAL0, 176, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(4, &item_corridor_lamp_sprite, PAL0, 336, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(5, &item_corridor_lamp_sprite, PAL0, 464, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(6, &item_corridor_lamp_sprite, PAL0, 656, 0, 0, 0, 0, 0, FORCE_BACKGROUND);
    init_item(7, &item_corridor_door_bottom_sprite, PAL0, 96, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(8, &item_corridor_door_bottom_sprite, PAL0, 400, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(9, &item_corridor_door_bottom_sprite, PAL0, 720, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(10, &item_corridor_map_bottom_sprite, PAL0, 240, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);
    init_item(11, &item_corridor_map_bottom_sprite, PAL0, 544, 120, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, COLLISION_DEFAULT, FORCE_BACKGROUND);

    init_character(CHR_linus);
}

static void act1_scene2_entry(void)    // Entrada de Linus por la derecha
{
    move_character_instant(CHR_linus, 340, 154);
    move_character(CHR_linus, 270, 154);
}

static void act1_scene2_items(void)    // Bucle de libros/puertas/mapas + condición de salida + salida
{
    bool item_interacted[2] = {false, false}; // Books read

    while (true) {
        switch (last_interacted_item)
        {
        case 0: // Guild history book
            if (item_interacted[0] == false) talk_cluster(&dialogs[ACT1_DIALOG1][A1D1_BOOK_HISTORY], false);
            else talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_BOOK_HISTORY], false);
            item_interacted[0] = true;
            last_interacted_item = ITEM_NONE;
            break;
        case 1: // Myths and legends
            if (item_interacted[1] == false) talk_cluster(&dialogs[ACT1_DIALOG1][A1D1_MYTH_COLLECTION], false);
            else talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_MYTH_COLLECTION], false);
            item_interacted[1] = true;
            last_interacted_item = ITEM_NONE;
            break;
        case 7: // Left door
            talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_DOOR1_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 8: // Middle door
            talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_DOOR2_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 9: // Right door
            talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_DOOR3_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        case 10: // Map
        case 11: // Map
            talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_MAP_CHECK], false);
            last_interacted_item = ITEM_NONE;
            break;
        default:
            break;
        }

        // El jugador intenta salir por la izquierda
        if (FASTFIX32_TO_INT(offset_BGA) <= 1 && FASTFIX32_TO_INT(obj_character[active_character].x) <= 1) {
            if (item_interacted[0] == false || item_interacted[1] == false) { // Sin leer ambos libros
                talk_dialog(&dialogs[ACT1_DIALOG1][A1D1_REVISIT_MEMORIES], false);
                move_character(active_character,
                    20,
                    FASTFIX32_TO_INT(obj_character[active_character].y) +
                    obj_character[active_character].y_size); // Go backwards
            }
            else break; // Leídos --> salir
        }

        next_frame(true);
    }

    // Salida por la izquierda
    move_character(active_character,
        -30,
        FASTFIX32_TO_INT(obj_character[active_character].y) +
        obj_character[active_character].y_size);
}

// =====================================================================
// ESCENA 3 — Hall con Clio y Xander
// =====================================================================

static void act1_scene3_setup(void)    // Nivel del hall + posiciones iniciales
{
    new_level(&historians_bg_tile, &historians_bg_map, &historians_front_tile, &historians_front_map, historians_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 3);

    init_character(CHR_linus);
    init_character(CHR_clio);
    init_character(CHR_xander);

    move_character_instant(CHR_clio, 40, 174);
    move_character_instant(CHR_linus, 360, 164);
    look_left(CHR_clio, false);
    look_left(CHR_linus, true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
}

// =====================================================================
// ESCENA 5 — Bosque: tutorial + combate
// =====================================================================

static void act1_scene5_setup(void)    // Nivel del bosque + items + personajes + hechizos del tutorial
{
    new_level(&forest_bg_tile, &forest_bg_map, &forest_front_tile, &forest_front_map, forest_dark_pal, 1440, BG_SCRL_USER_RIGHT, 3);
    set_limits(0,134,275,172);

    init_item(0, &item_forest_fg1_sprite, PAL0, 260, 176-16, 0, 0, 0, 0, FORCE_FOREGROUND); // Rock 1
    init_item(1, &item_forest_fg5_sprite, PAL0, 180, 0, 0, 0, 0, 0, FORCE_FOREGROUND); // Tree (vertical)
    init_item(2, &item_forest_fg2_sprite, PAL0, 440, 176-24, 0, 0, 0, 0, FORCE_FOREGROUND); // Rock 2
    init_item(3, &item_forest_fg3_sprite, PAL0, 880, 176-16, 0, 0, 0, 0, FORCE_FOREGROUND); // Rock 3
    init_item(4, &item_forest_fg4_sprite, PAL0, 1400, 176-72, 0, 0, 0, 0, FORCE_FOREGROUND); // Tree (big)
    init_item(5, &item_forest_fg1_sprite, PAL0, 1050, 176-16, 0, 0, 0, 0, FORCE_FOREGROUND); // Rock 1
    init_item(6, &item_forest_fg5_sprite, PAL0, 1270, 0, 0, 0, 0, 0, FORCE_FOREGROUND); // Tree (vertical)

    player_has_rod = true;
    init_character(CHR_linus);
    init_character(CHR_clio);
    active_character = CHR_linus;
    move_character_instant(CHR_linus, -30, 154);
    move_character_instant(CHR_clio, -30, 154);
    follow_active_character(CHR_clio, true); // Clio follows Linus

    spell_enable(SPELL_THUNDER);
    spell_enable(SPELL_HIDE);
    spell_enable(SPELL_OPEN);
    spell_enable(SPELL_SLEEP);

    move_character(CHR_linus, SCROLL_START_DISTANCE+10, 154);

    // Stop protagonist before talking
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    SPR_update();
}

static void act1_scene5_pad_hint(void)    // Aviso si el jugador usa un mando de 3 botones
{
    dprintf(2,"Detected controller type: %d\n", JOY_getJoypadType(JOY_1));
    if (JOY_getJoypadType(JOY_1) == JOY_TYPE_PAD3) {
        talk_cluster(&dialogs[ACT1_DIALOG3][A1D3_3BUTTONS], true);
    }
}

static void act1_scene5_day(void)    // Fade a la paleta de día del bosque
{
    PAL_fadeTo(0, 15, forest_pal.data, SCREEN_FPS, false);
}

static void act1_scene5_enemies(void)    // Aparición de los dos WeaverGhosts (previo al op combat)
{
    show_or_hide_interface(false);

    PAL_setPalette(PAL3, weaver_ghost_sprite.palette->data, DMA); // Enemy palette

    // Stop everybody before ghosts appear
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
    reset_character_animations();
    SPR_update();

    init_enemy(0, ENEMY_CLS_WEAVERGHOST);
    init_enemy(1, ENEMY_CLS_WEAVERGHOST);
    move_enemy_instant(0, FASTFIX32_FROM_INT(350), FASTFIX32_FROM_INT(176));
    move_enemy_instant(1, FASTFIX32_FROM_INT(-20), FASTFIX32_FROM_INT(156));
    move_enemy(0, FASTFIX32_FROM_INT(250), FASTFIX32_FROM_INT(136));
    move_enemy(1, FASTFIX32_FROM_INT(20), FASTFIX32_FROM_INT(156));
}

// =====================================================================
// Tabla de despacho (indexada por SceneHookId — mantener el orden del enum)
// =====================================================================

const SceneHook scene_hook_table[HOOK_COUNT] = {
    [HOOK_ACT1_SCENE1_SETUP]    = act1_scene1_setup,
    [HOOK_ACT1_SCENE1_SWAN]     = act1_scene1_swan,
    [HOOK_ACT1_SCENE1_WAKE]     = act1_scene1_wake,
    [HOOK_ACT1_SCENE1_ITEMS]    = act1_scene1_items,
    [HOOK_ACT1_SCENE2_SETUP]    = act1_scene2_setup,
    [HOOK_ACT1_SCENE2_ENTRY]    = act1_scene2_entry,
    [HOOK_ACT1_SCENE2_ITEMS]    = act1_scene2_items,
    [HOOK_ACT1_SCENE3_SETUP]    = act1_scene3_setup,
    [HOOK_ACT1_SCENE5_SETUP]    = act1_scene5_setup,
    [HOOK_ACT1_SCENE5_PAD_HINT] = act1_scene5_pad_hint,
    [HOOK_ACT1_SCENE5_DAY]      = act1_scene5_day,
    [HOOK_ACT1_SCENE5_ENEMIES]  = act1_scene5_enemies,
};
