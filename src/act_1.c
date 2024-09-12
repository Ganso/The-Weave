#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)
{
    // Initialize level
    new_level(NULL, NULL, &historians_corridor_front_tile, &historians_corridor_front_map, historians_corridor_pal, 576, BG_SCRL_USER_LEFT, 0);
    set_limits(0,131,275,170);

    // Initialize items
    init_item(0, &item_bookpedestal_sprite, PAL0, 400, 90, 0, 0, 8, 58); // Guild history book
    init_item(1, &item_bookpedestal_sprite, PAL0, 200, 90, 0, 0, 8, 58); // Myths and legends

    // Initialize characters and dialog faces
    init_character(CHR_linus);

    // Put character in screen
    move_character_instant(CHR_linus, 340, 154);
    move_character(CHR_linus, 270, 154);

    talk_dialog(&dialogs[ACT1_DIALOG1][0]);
    talk_dialog(&dialogs[ACT1_DIALOG1][1]);

    // You can move
    player_scroll_active=true;
    movement_active=true;

    bool item_interacted[2]={false, false};
    while (true) {
        switch (pending_item_interaction) // Process item interactions
        {
        case 0: // Guild history book
            talk_dialog(&dialogs[ACT1_DIALOG1][3]);
            talk_dialog(&dialogs[ACT1_DIALOG1][4]);
            talk_dialog(&dialogs[ACT1_DIALOG1][5]);
            item_interacted[0]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        case 1: // Myths and legends
            talk_dialog(&dialogs[ACT1_DIALOG1][6]);
            talk_dialog(&dialogs[ACT1_DIALOG1][7]);
            item_interacted[1]=true;
            pending_item_interaction=ITEM_NONE;
            break;
        default:
            break;
        }

        if (offset_BGA<=1 && obj_character[active_character].x<=1) { // Players try to exit screen
            if (item_interacted[0]==false || item_interacted[1]==false) { // We han't read every book
                talk_dialog(&dialogs[ACT1_DIALOG1][2]);
                move_character(active_character,20,obj_character[active_character].y+obj_character[active_character].y_size); // Go backwards
            }
            else break; // We have read it --> exit
        }

        next_frame(true);
    }

    move_character(active_character,-30,obj_character[active_character].y+obj_character[active_character].y_size);

    current_scene=2; // Next scene
}

void act_1_scene_2(void)
{
    u16 ndialog;
    const DialogItem *current_dialog;

    // Initialize level
    new_level(&historians_bg_tile, &historians_bg_map, &historians_front_tile, &historians_front_map, historians_pal, SCREEN_WIDTH, BG_SCRL_AUTO_RIGHT, 2);
    
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
    
    // Dialog
    ndialog=0;
    while ((current_dialog = &dialogs[ACT1_DIALOG2][ndialog])->text[game_language] != NULL) {
        switch(ndialog) {
            case 3:
                // After Clio's first line
                move_character(CHR_linus, 200, 174);
                break;
            case 6:
                // Xander's entrance
                move_character(CHR_clio, 100, 154);
                wait_seconds(1);
                look_left(CHR_clio, true);
                move_character_instant(CHR_xander,-30,174);
                show_character(CHR_xander, true);
                move_character(CHR_xander, 40, 174);
                break;
            case 20:
                // Before Clio's last line
                look_left(CHR_clio, false);
                break;
        }
        talk_dialog(current_dialog);
        ndialog++;
    }

    wait_seconds(2);

    current_scene=5; // Next scene
}

void act_1_scene_5(void)
{
    // Initialize level
    new_level(&weavers_bg_tile, &weavers_bg_map, &weavers_front_tile, &weavers_front_map, weavers_pal, 1000, BG_SCRL_USER_RIGHT, 3);
    set_limits(0,134,275,172);

    // Initialize characters
    init_character(CHR_linus);
    active_character=CHR_linus;

    // Dialog
    move_character_instant(CHR_linus, -20, 154);
    move_character(CHR_linus, 30, 154);
    talk_dialog(&dialogs[ACT1_DIALOG3][0]);
    talk_dialog(&dialogs[ACT1_DIALOG3][1]);

    // Show the interface and allow character to move and play patterns
    player_scroll_active=true;
    movement_active=true;
    interface_active=true;
    patterns_enabled=true;
    show_or_hide_interface(true);

    while (offset_BGA<80) {
        next_frame(true);
    }

    // COMBAT SCENE
    show_or_hide_interface(false);

    // Initialize enemies
    PAL_setPalette(PAL3, three_head_monkey_sprite.palette->data, DMA); // Enemy palette

    init_enemy(0,ENEMY_CLS_BADBOBBIN);
    move_enemy_instant(0, 350, 176);
    move_character(CHR_linus, 200, 144);
    move_enemy(0, 250, 136);

    show_or_hide_interface(true);
    start_combat(true);

    while (is_combat_active==true) {
        next_frame(true);
    }

    init_enemy(0,ENEMY_CLS_3HEADMONKEY);
    init_enemy(1,ENEMY_CLS_BADBOBBIN);
    move_enemy_instant(0, -20, 156);
    move_enemy_instant(1, 350, 156);
    move_enemy(0, 20, 156);
    move_enemy(1, 250, 156);
    start_combat(true);

    while (true) {
        next_frame(true);
    }
}