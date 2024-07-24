#include <genesis.h>
#include "globals.h"

void initialize(void)
{
    // Initialize VPD
    VDP_init();

    // Initialize audio driver
    Z80_init();
    Z80_loadDriver(Z80_DRIVER_XGM, 1);

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
    PAL_setPalette(PAL1, linus_sprite.palette->data, DMA); // Characters palette
    PAL_setPalette(PAL2, interface_pal.data, DMA); // Interface palette

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
}
