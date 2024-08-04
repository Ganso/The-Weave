#include <genesis.h>
#include "globals.h"

void initialize(void)
{
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
    game_language=LANG_ENGLISH;

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
    init_patterns();

    // Enemys
    init_enemy_classes();
    init_enemy_patterns();
}

// initialize level and load background
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

    interface_active=false; // No interface by default
    player_scroll_active=false; // You can scroll the screen by default
    movement_active=false; // You can't move by default

    update_bg();
}
