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
        SPR_setVisibility(spr_int_rod, VISIBLE);
        SPR_setVisibility(spr_int_pentagram, VISIBLE);
    } 
    else {
        SPR_setVisibility(spr_int_rod, HIDDEN);
        SPR_setVisibility(spr_int_rod_1, HIDDEN);
        SPR_setVisibility(spr_int_rod_2, HIDDEN);
        SPR_setVisibility(spr_int_rod_3, HIDDEN);
        SPR_setVisibility(spr_int_rod_4, HIDDEN);
        SPR_setVisibility(spr_int_rod_5, HIDDEN);
        SPR_setVisibility(spr_int_rod_6, HIDDEN);
        SPR_setVisibility(spr_int_pentagram, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_1, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_2, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_3, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_4, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_5, HIDDEN);
        SPR_setVisibility(spr_int_pentagram_6, HIDDEN);
    }
}