#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)
{
    new_level(historians_bg_tile, historians_bg_map, historians_front_tile, historians_front_map, historians_pal, BG_SCRL_AUTO_LEFT, 3);

    look_left(CHR_clio,false);
    look_left(CHR_linus,true);

    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    talk_dialog(FACE_none, SIDE_none, ACT1_DIALOG2, 0, 0);
    talk_dialog(FACE_none, SIDE_none, ACT1_DIALOG2, 1, 0);
    move_character(CHR_linus, 200, 110);
    talk_dialog(FACE_clio, SIDE_left, ACT1_DIALOG1, 0, 0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 1,  0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 2, 0);
    talk_dialog(FACE_clio, SIDE_left, ACT1_DIALOG1, 3, 0);
    move_character(CHR_clio, 100, 90);
    wait_seconds(1);
    look_left(CHR_clio, true);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 110);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 4, 0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 5, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 6, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 7, 0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 8, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 9, 0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 10, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 11, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 12, 0);
    talk_dialog(FACE_clio, SIDE_right, ACT1_DIALOG1, 13, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 14, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 15, 0);
    talk_dialog(FACE_xander, SIDE_left, ACT1_DIALOG1, 16, 0);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 17, 100);
    talk_dialog(FACE_linus, SIDE_right, ACT1_DIALOG1, 18, 0);
    look_left(CHR_clio, false);
    talk_dialog(FACE_clio, SIDE_left, ACT1_DIALOG1, 19, 0);

    wait_seconds(2);

    act_1_scene_5();
}



void act_1_scene_5(void)
{
    new_level(weavers_bg_tile, weavers_bg_map, weavers_front_tile, weavers_front_map, weavers_pal, BG_SCRL_USER_RIGHT, 3);
    set_limits(20,70,270,108);

    active_character=CHR_linus;
    talk_dialog(FACE_none, SIDE_none, ACT1_DIALOG3, 0, 0);
    move_character_instant(CHR_linus, -20, 90);
    move_character(CHR_linus, 30, 90);
    talk_dialog(FACE_linus, SIDE_left, ACT1_DIALOG3, 1, 0);

    while (offset_BGA<200) {
        joy_check();
        next_frame();
    }

    talk_dialog(FACE_linus, SIDE_left, ACT1_DIALOG3, 1, 0);

     while (1) {
        joy_check();
        next_frame();
    }
   
}