#include <genesis.h>
#include "globals.h"

void initialize(void)
{
    u8 counter;

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

    // Character sprites
    // * Sprite definition, x, y, palette, priority, flipH, animation
    obj_character[CHR_linus] = (Entity) { &linus_sprite, 400, 100, PAL1, false, false, ANIM_IDLE, false };
    obj_character[CHR_clio] = (Entity) { &clio_sprite, 40, 110, PAL1, false, false, ANIM_IDLE, false };
    obj_character[CHR_xander] = (Entity) { &xander_sprite, -40, 110, PAL1, false, false, ANIM_IDLE, false };
    for (counter=0; counter<MAX_CHR; counter++) initialize_character(counter);

    // Character faces
    // * Sprite definition, x, y, palette, priority, flipH, animation
    obj_face[FACE_linus] = (Entity) { &linus_face_sprite, 0, 160, PAL1, false, false, ANIM_IDLE, false };
    obj_face[FACE_clio] = (Entity) { &clio_face_sprite, 0, 160, PAL1, false, false, ANIM_IDLE, false };
    obj_face[FACE_xander] = (Entity) { &xander_face_sprite, 0, 160, PAL1, false, false, ANIM_IDLE, false };
    for (counter=0; counter<MAX_FACE; counter++) initialize_face(counter);

    // Interface: Face backgrounds
    spr_face_left = SPR_addSprite ( &face_left_sprite, 0, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_left, HIDDEN);
    spr_face_right = SPR_addSprite ( &face_right_sprite, 256, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setDepth (spr_face_left, SPR_MIN_DEPTH+1); // Face background are above anything but faces
    SPR_setDepth (spr_face_right, SPR_MIN_DEPTH+1); // Face background are above anything but faces

    // Interface: Button A
    spr_button_A = SPR_addSprite (&button_A_sprite, 0, 0, TILE_ATTR(PAL2, false, false, false));
    SPR_setVisibility (spr_button_A, HIDDEN);

}
