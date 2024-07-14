#include <genesis.h>
#include "globals.h"

void act_1_scene_1(void)
{
    VDP_loadTileSet(&historians_front_tile, tile_ind, DMA);
    background_BGA = MAP_create(&historians_front_map, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += historians_front_tile.numTile;

    VDP_loadTileSet(&historians_bg_tile, tile_ind, DMA);
    background_BGB = MAP_create(&historians_bg_map, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += historians_bg_tile.numTile;

    background_scroll_mode=BG_SCRL_AUTO_LEFT;
    background_speed=3;

    PAL_setPalette(PAL0, historians_pal.data, DMA); // Backgrounds palette

    MAP_scrollTo(background_BGA, 0, 0);
    MAP_scrollTo(background_BGB, 0, 0);

    update_bg();

    // Conversation
    look_left(CHR_clio,false);
    look_left(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    talk_dialog(FACE_none, true, ACT1_DIALOG2, 0, 0);
    talk_dialog(FACE_none, true, ACT1_DIALOG2, 1, 0);
    move_character(CHR_linus, 200, 110);
    talk_dialog(FACE_clio, true, ACT1_DIALOG1, 0, 0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 1,  0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 2, 0);
    talk_dialog(FACE_clio, true, ACT1_DIALOG1, 3, 0);
    move_character(CHR_clio, 100, 90);
    wait_seconds(1);
    look_left(CHR_clio, true);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 110);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 4, 0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 5, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 6, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 7, 0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 8, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 9, 0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 10, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 11, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 12, 0);
    talk_dialog(FACE_clio, false, ACT1_DIALOG1, 13, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 14, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 15, 0);
    talk_dialog(FACE_xander, true, ACT1_DIALOG1, 16, 0);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 17, 100);
    talk_dialog(FACE_linus, false, ACT1_DIALOG1, 18, 0);
    look_left(CHR_clio, false);
    talk_dialog(FACE_clio, true, ACT1_DIALOG1, 19, 0);

    wait_seconds(2);

    act_1_scene_5();
}



void act_1_scene_5(void)
{
    init();

    VDP_loadTileSet(&weavers_front_tile, tile_ind, DMA);
    background_BGA = MAP_create(&weavers_front_map, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += weavers_front_tile.numTile;

    VDP_loadTileSet(&weavers_bg_tile, tile_ind, DMA);
    background_BGB = MAP_create(&weavers_bg_map, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += weavers_bg_tile.numTile;

    PAL_setPalette(PAL0, weavers_pal.data, DMA); // Backgrounds palette

    background_scroll_mode=BG_SCRL_USER_RIGHT;
    background_speed=3;

    MAP_scrollTo(background_BGA, 0, 0);
    MAP_scrollTo(background_BGB, 0, 0);
    offset_BGA=0;
    offset_BGB=0;

    x_limit_min=20;
    y_limit_min=70;
    x_limit_max=270;
    y_limit_max=108;

    active_character=CHR_linus;
    talk_dialog(FACE_none, false, ACT1_DIALOG3, 0, 0);
    move_character_instant(CHR_linus, -20, 90);
    move_character(CHR_linus, 30, 90);
    talk_dialog(FACE_linus, true, ACT1_DIALOG3, 1, 0);

    while (1) {
        joy_check();
        next_frame();
    }
}