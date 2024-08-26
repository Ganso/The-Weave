#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)
{
    u16 ndialog;
    const DialogItem *current_dialog;

    // Initialize level
    new_level(historians_bg_tile, historians_bg_map, historians_front_tile, historians_front_map, historians_pal, BG_SCRL_AUTO_LEFT, 2);

    // Initialize characters and dialog faces
    init_character(CHR_linus);
    init_character(CHR_clio);
    init_character(CHR_xander);
    
    // Starting positions
    move_character_instant(CHR_clio,20,174);
    move_character_instant(CHR_linus,360,154);
    look_left(CHR_clio,false);
    look_left(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    
    // Dialog - Introduction
    ndialog=0;
    while ((current_dialog = &dialogs[ACT1_DIALOG1][ndialog])->text[game_language] != NULL) {
        talk_dialog(current_dialog);
        ndialog++;
    }

    // Main dialog
    ndialog = 0;
    while ((current_dialog = &dialogs[ACT1_DIALOG2][ndialog])->text[game_language] != NULL) {
        switch(ndialog) {
            case 1:
                // After Clio's first line
                move_character(CHR_linus, 200, 174);
                break;
            case 4:
                // Xander's entrance
                move_character(CHR_clio, 100, 154);
                wait_seconds(1);
                look_left(CHR_clio, true);
                move_character_instant(CHR_xander,-30,174);
                show_character(CHR_xander, true);
                move_character(CHR_xander, 40, 174);
                break;
            case 18:
                // Before Clio's last line
                look_left(CHR_clio, false);
                break;
        }
        talk_dialog(current_dialog);
        ndialog++;
    }

    wait_seconds(2);

    act_1_scene_5();
}

void act_1_scene_5(void)
{
    u16 ndialog;
    const DialogItem *current_dialog;

    // Initialize level
    new_level(weavers_bg_tile, weavers_bg_map, weavers_front_tile, weavers_front_map, weavers_pal, BG_SCRL_USER_RIGHT, 3);
    set_limits(20,70,270,108);

    // Initialize characters
    init_character(CHR_linus);
    active_character=CHR_linus;

    // Dialog
    ndialog=0;
    while ((current_dialog = &dialogs[ACT1_DIALOG3][ndialog])->text[game_language] != NULL) {
        switch(ndialog) {
            case 1:
                // Before Linus' line
                move_character_instant(CHR_linus, -20, 154);
                move_character(CHR_linus, 30, 154);
                break;
        }

        talk_dialog(current_dialog);
        ndialog++;
    }

    // Show the interface and allow character to move
    player_scroll_active=true;
    movement_active=true;
    interface_active=true;
    show_interface(true);

    while (offset_BGA<80) {
        next_frame();
    }

    // COMBAT SCENE
    show_interface(false);

    // Initialize enemies
    PAL_setPalette(PAL3, three_head_monkey_sprite.palette->data, DMA); // Enemy palette

    init_enemy(0,ENEMY_CLS_BADBOBBIN);
    move_enemy_instant(0, 350, 176);
    move_character(CHR_linus, 200, 144);
    move_enemy(0, 250, 136);

    show_interface(true);
    start_combat(true);

    while (is_combat_active==true) {
        next_frame();
    }

    init_enemy(0,ENEMY_CLS_3HEADMONKEY);
    init_enemy(1,ENEMY_CLS_BADBOBBIN);
    move_enemy_instant(0, -20, 156);
    move_enemy_instant(1, 350, 156);
    move_enemy(0, 20, 156);
    move_enemy(1, 250, 156);
    start_combat(true);

    while (is_combat_active==true) {
        next_frame();
    }
}