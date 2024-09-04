#include <genesis.h>
#include "globals.h"

void initialize(void)
{
    u8 i;

    // Initialize VPD
    VDP_init();

    // Initialize audio driver
    Z80_init();
    Z80_loadDriver(Z80_DRIVER_XGM2, 1);

    // Initialize sprite Engine
    SPR_init();

    // Initialize controllers
    JOY_init();

    // Screen definitions
    VDP_setScreenWidth320();
    VDP_setScreenHeight224();

    // Load font
    VDP_loadFont(font.tileset, DMA);

    // Initialize globals
    tile_ind = TILE_USER_INDEX;

    // Default language
    game_language=LANG_SPANISH;

    //  Plane A scrolls up to line 22 (176px)
    VDP_setWindowVPos(TRUE, 22);

    // Initialize palettes
    // PAL0 is the background palette. It's initialized with the background
    PAL_setPalette(PAL1, linus_sprite.palette->data, DMA); // Characters palette
    PAL_setPalette(PAL2, interface_pal.data, DMA); // Interface palette
    // PAL2 is the enemies palette. It's initialized with the enemies

    // Interface: Face backgrounds
    spr_face_left = SPR_addSpriteSafe ( &face_left_sprite, 0, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_left, HIDDEN);
    spr_face_right = SPR_addSpriteSafe ( &face_right_sprite, 256, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setDepth (spr_face_left, SPR_MIN_DEPTH+1); // Face background are above anything but faces
    SPR_setDepth (spr_face_right, SPR_MIN_DEPTH+1); // Face background are above anything but faces

    // Interface: Button A
    spr_int_button_A = SPR_addSpriteSafe (&int_button_A_sprite, 0, 0, TILE_ATTR(PAL2, false, false, false));
    SPR_setVisibility (spr_int_button_A, HIDDEN);

    // Notes and patterns;
    note_playing=0;
    note_playing_time=0;
    num_played_notes=0;
    time_since_last_note=0;
    pattern_effect_in_progress=PTRN_NONE;
    pattern_effect_reversed=false;
    pattern_effect_time=0;
    init_patterns();

    // Enemys and combat
    init_enemy_classes();
    init_enemy_patterns();
    is_combat_active=false;

    // Mark all entities as inactive
    for (i=0;i<MAX_CHR;i++) obj_character[i].active=false;
    for (i=0;i<MAX_FACE;i++) obj_face[i].active=false;
    for (i=0;i<MAX_ENEMIES;i++) obj_enemy[i].obj_character.active=false;
}

// initialize level and load background
void new_level(const TileSet *tile_bg, const MapDefinition *map_bg, const TileSet *tile_front, const MapDefinition *map_front, Palette new_pal, u16 new_background_width, u8 new_scroll_mode, u8 new_scroll_speed)
{
    initialize();
    
    // Tile_bg and Map_bg are the background layer. They can be NULL
    if ((tile_bg!=NULL) && (map_bg!=NULL)) {
        VDP_loadTileSet(tile_bg, tile_ind, DMA);
        background_BGB = MAP_create(map_bg, BG_B, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
        tile_ind += tile_bg->numTile;
    }
    else background_BGB=NULL;

    // Tile_front and Map_front are the foreground layer. Thay can't be NULL.
    VDP_loadTileSet(tile_front, tile_ind, DMA);
    background_BGA = MAP_create(map_front, BG_A, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind));
    tile_ind += tile_front->numTile;

    PAL_setPalette(PAL0, new_pal.data, DMA);

    background_scroll_mode=new_scroll_mode;
    scroll_speed=new_scroll_speed;
    background_width=new_background_width;

    offset_BGA=0;
    offset_BGB=0;

    if (background_scroll_mode==BG_SCRL_USER_LEFT) { // We should start at the rightmost edge of the screen
        offset_BGA=background_width-SCREEN_WIDTH;
    }

    interface_active=false; // No interface by default
    player_scroll_active=false; // You can scroll the screen by default
    movement_active=false; // You can't move by default

    update_bg(false);
}
