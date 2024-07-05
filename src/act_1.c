#include <genesis.h>
#include "globals.h"

#include "act_1.h"

void start_act_1(void)
{
    VDP_loadTileSet(&historians_front_tile, tile_ind, DMA);
    bg_historians_front = MAP_create(&historians_front_map, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += historians_front_tile.numTile;

    VDP_loadTileSet(&historians_bg_tile, tile_ind, DMA);
    bg_historians_back = MAP_create(&historians_bg_map, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += historians_bg_tile.numTile;

    background_BGA=bg_historians_front;
    background_BGB=bg_historians_back;

    MAP_scrollTo(background_BGA, 0, 0);
    MAP_scrollTo(background_BGB, 0, 0);

    update_bg();

    // Conversation
    look_left(CHR_clio,false);
    look_left(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    move_character(CHR_linus, 200, 110);
    talk_dialog(FACE_clio, true, ACT1_SEQ1, 0, 0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 1,  0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 2, 0);
    talk_dialog(FACE_clio, true, ACT1_SEQ1, 3, 0);
    move_character(CHR_clio, 100, 90);
    wait_seconds(1);
    look_left(CHR_clio, true);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 110);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 4, 0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 5, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 6, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 7, 0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 8, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 9, 0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 10, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 11, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 12, 0);
    talk_dialog(FACE_clio, false, ACT1_SEQ1, 13, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 14, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 15, 0);
    talk_dialog(FACE_xander, true, ACT1_SEQ1, 16, 0);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 17, 100);
    talk_dialog(FACE_linus, false, ACT1_SEQ1, 18, 0);
    look_left(CHR_clio, false);
    talk_dialog(FACE_clio, true, ACT1_SEQ1, 19, 0);

    wait_seconds(5);
    SYS_reset();

    while (1)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
}

