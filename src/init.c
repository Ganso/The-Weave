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
    game_language=LANG_SPANISH;

    //  Plane A scrolls up to line 22 (176px)
    VDP_setWindowVPos(TRUE, 22);

    // Initialize palettes
    PAL_setPalette(PAL1, linus_sprite.palette->data, DMA); // Characters palette
    PAL_setPalette(PAL2, interface_pal.data, DMA); // Interface palette

    // Interface: Face backgrounds
    spr_face_left = SPR_addSprite ( &face_left_sprite, 0, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_left, HIDDEN);
    spr_face_right = SPR_addSprite ( &face_right_sprite, 256, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setDepth (spr_face_left, SPR_MIN_DEPTH+1); // Face background are above anything but faces
    SPR_setDepth (spr_face_right, SPR_MIN_DEPTH+1); // Face background are above anything but faces

    // Interface: Button A
    spr_int_button_A = SPR_addSprite (&int_button_A_sprite, 0, 0, TILE_ATTR(PAL2, false, false, false));
    SPR_setVisibility (spr_int_button_A, HIDDEN);

    // Interface: Rod & Pentagram
    spr_int_rod = SPR_addSprite (&int_rod_sprite, 4, 190, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram = SPR_addSprite (&int_pentagram_sprite, 219, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_1 = SPR_addSprite (&int_pentagram_1_sprite, 219, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_2 = SPR_addSprite (&int_pentagram_2_sprite, 219+16, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_3 = SPR_addSprite (&int_pentagram_3_sprite, 219+32, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_4 = SPR_addSprite (&int_pentagram_4_sprite, 219+48, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_5 = SPR_addSprite (&int_pentagram_5_sprite, 219+64, 182, TILE_ATTR(PAL2, false, false, false));
    spr_int_pentagram_6 = SPR_addSprite (&int_pentagram_6_sprite, 219+80, 182, TILE_ATTR(PAL2, false, false, false));
    SPR_setDepth(spr_int_pentagram_1, SPR_MIN_DEPTH);
    SPR_setDepth(spr_int_pentagram_2, SPR_MIN_DEPTH);
    SPR_setDepth(spr_int_pentagram_3, SPR_MIN_DEPTH);
    SPR_setDepth(spr_int_pentagram_4, SPR_MIN_DEPTH);
    SPR_setDepth(spr_int_pentagram_5, SPR_MIN_DEPTH);
    SPR_setDepth(spr_int_pentagram_6, SPR_MIN_DEPTH);
    show_interface(false);
}
