#include <genesis.h>
#include "globals.h"

#include "act_1.h"

// Maps
Map *bg_historians = NULL; // Historians background

void start_act_1(void)
{
    VDP_loadTileSet(&historians_tile, tile_ind, DMA);
    bg_historians = MAP_create(&historians_map, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += historians_tile.numTile;

    MAP_scrollTo(bg_historians, 0, 0);

    SPR_update();
    SYS_doVBlankProcess();

    // Conversation
    look_left(CHR_clio,false);
    look_left(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    move_character(CHR_linus, 200, 110);
    talk_text(FACE_clio, true, ACT1_SEQ1, 0, 0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 1,  0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 2, 0);
    talk_text(FACE_clio, true, ACT1_SEQ1, 3, 0);
    move_character(CHR_clio, 100, 90);
    wait_seconds(1);
    look_left(CHR_clio, true);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 110);
    talk_text(FACE_xander, true, ACT1_SEQ1, 4, 0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 5, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 6, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 7, 0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 8, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 9, 0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 10, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 11, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 12, 0);
    talk_text(FACE_clio, false, ACT1_SEQ1, 13, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 14, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 15, 0);
    talk_text(FACE_xander, true, ACT1_SEQ1, 16, 0);
    talk_text(FACE_linus, false, ACT1_SEQ1, 17, 100);
    talk_text(FACE_linus, false, ACT1_SEQ1, 18, 0);
    look_left(CHR_clio, false);
    talk_text(FACE_clio, true, ACT1_SEQ1, 19, 0);

    wait_seconds(5);
    SYS_reset();

    while (1)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
}

