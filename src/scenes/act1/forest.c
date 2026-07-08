// act1/forest.c — hooks del bosque (lógica; la secuencia está en forest.scene)

#include <genesis.h>
#include "core/core.h"
#include "world/world.h"
#include "actors/actors.h"
#include "spells/spells.h"
#include "narrative/narrative.h"
#include "interface/interface.h"
#include "res_all.h"
#include "scenes/act1/forest.h"

void act1_forest_setup(void)    // Nivel del bosque + items + personajes + hechizos del tutorial
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

void act1_forest_pad_hint(void)    // Aviso si el jugador usa un mando de 3 botones
{
    dprintf(2,"Detected controller type: %d\n", JOY_getJoypadType(JOY_1));
    if (JOY_getJoypadType(JOY_1) == JOY_TYPE_PAD3) {
        talk_cluster(&dialogs[ACT1_FOREST][A1_FOREST_3BUTTONS], true);
    }
}

void act1_forest_day(void)    // Fade a la paleta de día del bosque
{
    PAL_fadeTo(0, 15, forest_pal.data, SCREEN_FPS, false);
}

void act1_forest_enemies(void)    // Aparición de los dos WeaverGhosts (previo al op combat)
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
