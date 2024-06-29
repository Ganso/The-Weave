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
    talk(FACE_clio, true, "It's late, Linus", "And you shouldn' be late", "at your birthday", 0);
    talk(FACE_linus, false, "I have had the strangest", "dream, mother", NULL, 0);
    talk(FACE_linus, false, "A swan came to my room", "and...", NULL, 0);
    talk(FACE_clio, true, "You can tell me later", "Xander is wating for us", NULL, 0);
    move_character(CHR_clio, 100, 90);
    wait_seconds(1);
    look_left(CHR_clio, true);
    show_character(CHR_xander, true);
    move_character(CHR_xander, 40, 110);
    talk(FACE_xander, true, "At last,", "you're awake, Linus", NULL, 0);
    talk(FACE_linus, false, "Forgive me, master", "A strange dream has", "kept me awake", 0);
    talk(FACE_xander, true, "You are certainly your", "father's son.", "Aiden had big dreams,", 0);
    talk(FACE_xander, true, "and we are here to talk", "about one that he", "never achieved", 0);
    talk(FACE_linus, false, "I've read his story a thousand times", "Which one is this?", NULL, 0);
    talk(FACE_xander, true, "One you won't find in a book", "The one about Weavers", "guild island", 0);
    talk(FACE_linus, false, "Weavers legend was always", "my favourite", NULL, 0);
    talk(FACE_xander, true, "Thas was no legend for him", "Shepards sang it as a fact", "Your father wanted to find it", 0);
    talk(FACE_clio, false, "Our destiny is to", "document facts", "Not to chase them", 0);
    talk(FACE_xander, true, "Linus is seventeen", "That was my age when", "I traveled the world", 0);
    talk(FACE_xander, true, "And his father's age when", "he came to us. A year before", "we took him as one of ours", 0);
    talk(FACE_linus, false, "Mother, I need to visit", "that island", NULL, 0);
    look_left(CHR_clio, false);
    talk(FACE_clio, true, "If Xander wants it that way,", "it will be so", "But you'll not go alone", 0);

    wait_seconds(5);
    SYS_reset();

    while (1)
    {
        SPR_update();
        SYS_doVBlankProcess();
    }
}

