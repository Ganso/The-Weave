#include <genesis.h>
#include "globals.h"

// Wait for N seconds
void wait_seconds(int sec)
{
    u16 num_ticks=0;
    u16 max_ticks=sec*60; // Ticks (at 60fps)
    while (num_ticks<max_ticks)
    {
        next_frame();
        num_ticks++;
    }

}

// Initialize a character
void initialize_character(u8 nchar)
{
    spr_chr[nchar] = SPR_addSprite ( obj_character[nchar].sd, obj_character[nchar].x, obj_character[nchar].y, TILE_ATTR(obj_character[nchar].palette, obj_character[nchar].priority, false, obj_character[nchar].flipH));
    SPR_setVisibility (spr_chr[nchar], HIDDEN);
}

// Initialize a face
void initialize_face(u8 nface)
{
    spr_face[nface] = SPR_addSprite ( obj_face[nface].sd, obj_face[nface].x, obj_face[nface].y, TILE_ATTR(obj_face[nface].palette, obj_face[nface].priority, false, obj_face[nface].flipH));
    SPR_setVisibility (spr_face[nface], HIDDEN);
    SPR_setDepth (spr_face[nface], SPR_MIN_DEPTH); // Faces are above any other sprite
}

// Update_background
void update_bg(void)
{
    if (background_scroll_mode==BG_SCRL_AUTO_LEFT) {
        MAP_scrollTo(background_BGB, offset_BGB>>background_speed, 0);
        offset_BGB++;
    }
}

// Wait for next frame
void next_frame(void)
{
    update_bg();
    SPR_update();
    SYS_doVBlankProcess();
}
