#include <genesis.h>
#include "globals.h"

// Update_background
void update_bg(void)
{
    if (background_scroll_mode==BG_SCRL_AUTO_LEFT) {
        MAP_scrollTo(background_BGB, offset_BGB>>scroll_speed, 0);
        offset_BGB++;
    }
}

// Initialice level and load background
void new_level(TileSet tile_bg, MapDefinition map_bg, TileSet tile_front, MapDefinition map_front, Palette new_pal, u8 new_scroll_mode, u8 new_scroll_speed)
{
    initialize();
    
    VDP_loadTileSet(&tile_bg, tile_ind, DMA);
    background_BGB = MAP_create(&map_bg, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += tile_bg.numTile;

    VDP_loadTileSet(&tile_front, tile_ind, DMA);
    background_BGA = MAP_create(&map_front, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += tile_front.numTile;

    background_scroll_mode=new_scroll_mode;
    scroll_speed=new_scroll_speed;

    PAL_setPalette(PAL0, new_pal.data, DMA);

    MAP_scrollTo(background_BGA, 0, 0);
    MAP_scrollTo(background_BGB, 0, 0);

    update_bg();
}

// Set background limits
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2)
{
    x_limit_min=x1;
    y_limit_min=y1;
    x_limit_max=x2;
    y_limit_max=y2;
}

// Show or hide the bottom interface
void show_interface(bool visible)
{
    if (visible == true) {
        spr_int_rod = SPR_addSpriteSafe (&int_rod_sprite, 4, 190, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_1 = SPR_addSpriteSafe (&int_rod_1_sprite, 24, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_2 = SPR_addSpriteSafe (&int_rod_2_sprite, 24+32, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_3 = SPR_addSpriteSafe (&int_rod_3_sprite, 24+64, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_4 = SPR_addSpriteSafe (&int_rod_4_sprite, 24+96, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_5 = SPR_addSpriteSafe (&int_rod_5_sprite, 24+128, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_rod_6 = SPR_addSpriteSafe (&int_rod_6_sprite, 24+160, 212, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram = SPR_addSpriteSafe (&int_pentagram_sprite, 219, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_1 = SPR_addSpriteSafe (&int_pentagram_1_sprite, 219, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_2 = SPR_addSpriteSafe (&int_pentagram_2_sprite, 219+16, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_3 = SPR_addSpriteSafe (&int_pentagram_3_sprite, 219+32, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_4 = SPR_addSpriteSafe (&int_pentagram_4_sprite, 219+48, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_5 = SPR_addSpriteSafe (&int_pentagram_5_sprite, 219+64, 180, TILE_ATTR(PAL2, false, false, false));
        spr_int_pentagram_6 = SPR_addSpriteSafe (&int_pentagram_6_sprite, 219+80, 180, TILE_ATTR(PAL2, false, false, false));
        SPR_setDepth(spr_int_pentagram_1, SPR_MIN_DEPTH);
        SPR_setDepth(spr_int_pentagram_2, SPR_MIN_DEPTH);
        SPR_setDepth(spr_int_pentagram_3, SPR_MIN_DEPTH);
        SPR_setDepth(spr_int_pentagram_4, SPR_MIN_DEPTH);
        SPR_setDepth(spr_int_pentagram_5, SPR_MIN_DEPTH);
        SPR_setDepth(spr_int_pentagram_6, SPR_MIN_DEPTH);
        SPR_setVisibility(spr_int_rod, VISIBLE);
        SPR_setVisibility(spr_int_pentagram, VISIBLE);
        SPR_setVisibility(spr_int_pentagram_1, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_2, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_3, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_4, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_5, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_6, HIDDEN);
        SPR_setVisibility(spr_int_rod_1,HIDDEN);
        SPR_setVisibility(spr_int_rod_2,HIDDEN);
        SPR_setVisibility(spr_int_rod_3,HIDDEN);
        SPR_setVisibility(spr_int_rod_4,HIDDEN);
        SPR_setVisibility(spr_int_rod_5,HIDDEN);
        SPR_setVisibility(spr_int_rod_6,HIDDEN);
    } 
    else {
        SPR_releaseSprite(spr_int_rod);
        SPR_releaseSprite(spr_int_rod_1);
        SPR_releaseSprite(spr_int_rod_2);
        SPR_releaseSprite(spr_int_rod_3);
        SPR_releaseSprite(spr_int_rod_4);
        SPR_releaseSprite(spr_int_rod_5);
        SPR_releaseSprite(spr_int_rod_6);
        SPR_releaseSprite(spr_int_pentagram);
        SPR_releaseSprite(spr_int_pentagram_1);
        SPR_releaseSprite(spr_int_pentagram_2);
        SPR_releaseSprite(spr_int_pentagram_3);
        SPR_releaseSprite(spr_int_pentagram_4);
        SPR_releaseSprite(spr_int_pentagram_5);
        SPR_releaseSprite(spr_int_pentagram_6);
    }
}