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
    look_right(CHR_clio,false);
    look_right(CHR_linus,true);
    show_character(CHR_clio, true);
    show_character(CHR_linus, true);
    wait_seconds(3);
    talk(FACE_clio, true, "It's late, Linus", "And you shouldn' be late", "at your birthday", 0);
    talk(FACE_linus, false, "I have had the strangest", "dream, mother.", NULL, 0);
    talk(FACE_linus, false, "A swan came to my room", "and...", NULL, 0);
    talk(FACE_clio, true, "You can tell me la", "Xander is wating for us", NULL, 0);

    while (1)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
}

