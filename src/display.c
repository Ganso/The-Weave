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

// Pause / State screen
void pause_screen(void) {
    u16 value; // Joypad value
    u8 selected_pattern,npattern;
    bool next_pattern_found;

    VDP_setHilightShadow(true); // Dim screen
    show_interface(false); // Hide interface
    
    selected_pattern=254;
    for (npattern=0;npattern<MAX_PATTERNS;npattern++)
        if (obj_pattern[npattern].active==true && selected_pattern==254) selected_pattern=npattern; // Find first active spell

    show_pattern_list(true,selected_pattern);

    value = JOY_readJoypad(JOY_ALL);
    while ( (value & BUTTON_START) == 0 ) { // Wait until button is pressed again
        value = JOY_readJoypad(JOY_ALL);
        if (value & BUTTON_RIGHT) { // Find next active pattern
            KDebug_Alert("RIGHT pressed");
            next_pattern_found=false;
            selected_pattern++;
            while (next_pattern_found==false)
            {
                if (selected_pattern==MAX_PATTERNS) selected_pattern=0;
                if (obj_pattern[selected_pattern].active==true) next_pattern_found=true;
                else selected_pattern++;
            }
            show_pattern_list(false,selected_pattern); // Show pattern list again
            show_pattern_list(true,selected_pattern);
        }
        if (value & BUTTON_LEFT) { // Find last active pattern
            KDebug_Alert("LEFT pressed");
            next_pattern_found=false;
            selected_pattern--;
            while (next_pattern_found==false)
            {
                if (selected_pattern==255) selected_pattern=MAX_PATTERNS-1;
                if (obj_pattern[selected_pattern].active==true) next_pattern_found=true;
                else selected_pattern--;
            }            
            show_pattern_list(false,selected_pattern); // Show pattern list again
            show_pattern_list(true,selected_pattern);
        }
        while ((value & BUTTON_LEFT) || (value & BUTTON_RIGHT)) // Wait until LEFT or RIGHT is released
        {
            KDebug_Alert("WAITING FOR RELEASE");
            value = JOY_readJoypad(JOY_ALL);
            next_frame();
        }
        next_frame();
    }
    while ( value & BUTTON_START ) { // Now wait until released
        value = JOY_readJoypad(JOY_ALL);
        next_frame();
    }

    show_pattern_list(false, 0);
    VDP_setHilightShadow(false); // Relit screen
    show_interface(true); // Show interface again
}

// Show or hide pattern list
void show_pattern_list(bool show, u8 active_pattern)
{
    u16 x_initial,x;
    u8 nnote, npattern, num_active_patterns=0;
    bool priority;

    KDebug_AlertNumber(active_pattern);

    for (npattern=0;npattern<MAX_PATTERNS;npattern++) // How many patterns you have?
        if (obj_pattern[npattern].active==true) num_active_patterns++;

    x_initial = (118 - 24 * num_active_patterns); // Initial X position
    x = x_initial;

    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        if (obj_pattern[npattern].active==true) {
            if (npattern==active_pattern) priority=true; // Lit active pattern
            else priority=false;
            show_pattern_icon(npattern,x,show,priority); // Show or hide pattern icon
            x+=48;
        }
    }

    for (nnote=0;nnote<4;nnote++) {
        show_note_in_pattern_list(active_pattern,nnote,show); // Show notes for the active pattern
    }
}

// Show one of the notes of a pattern in the pattern list
void show_note_in_pattern_list(u8 npattern, u8 nnote, bool show)
{
    SpriteDefinition *pentsprite;
    u8 note;
    u16 x;

    note=obj_pattern[npattern].notes[nnote]; // Which note is in that position of the pattern

    switch (note) 
    {
    case NOTE_MI:
        pentsprite=(SpriteDefinition*) &int_pentagram_1_sprite;
        break;
    case NOTE_FA:
        pentsprite=(SpriteDefinition*) &int_pentagram_2_sprite;
        break;
    case NOTE_SOL:
        pentsprite=(SpriteDefinition*) &int_pentagram_3_sprite;
        break;
    case NOTE_LA:
        pentsprite=(SpriteDefinition*) &int_pentagram_4_sprite;
        break;
    case NOTE_SI:
        pentsprite=(SpriteDefinition*) &int_pentagram_5_sprite;
        break;
    default:
        pentsprite=(SpriteDefinition*) &int_pentagram_6_sprite;
        break;
    }

    if (show==true) {
        x=219+nnote*16;
        spr_pattern_list_note[nnote]=SPR_addSpriteSafe(pentsprite, x, 180, TILE_ATTR(PAL2,false,false,false));
    }
    else {
        SPR_releaseSprite(spr_pattern_list_note[nnote]);
    }
}