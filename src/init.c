#include <genesis.h>
#include "globals.h"

#include "init.h"

void init(void)
{
    u8 counter;

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

    // Initialize globals
    tile_ind = TILE_USER_INDEX;

    // Initialize palettes
    PAL_setPalette(PAL0, bg_pal.data, DMA);
    PAL_setPalette(PAL1, linus_sprite.palette->data, DMA);

    // Character sprites
    // * Sprite definition, x, y, palette, priority, flipH, animation
    obj_character[CHR_linus] = (Entity) { &linus_sprite, 400, 110, PAL1, false, false, ANIM_IDLE, false };
    obj_character[CHR_clio] = (Entity) { &clio_sprite, 40, 110, PAL1, false, false, ANIM_IDLE, false };
    for (counter=0; counter<MAX_CHR; counter++) initialize_character(counter);

    // Character faces
    // * Sprite definition, x, y, palette, priority, flipH, animation
    obj_face[FACE_linus] = (Entity) { &linus_face_sprite, 0, 160, PAL1, false, false, ANIM_IDLE, false };
    obj_face[FACE_clio] = (Entity) { &clio_face_sprite, 0, 160, PAL1, false, false, ANIM_IDLE, false };
    for (counter=0; counter<MAX_FACE; counter++) initialize_face(counter);

    // Face backgrounds
    spr_face_left = SPR_addSprite ( &face_left_sprite, 0, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_left, HIDDEN);
    spr_face_right = SPR_addSprite ( &face_right_sprite, 256, 160, TILE_ATTR(PAL1, false, false, true));
    SPR_setVisibility (spr_face_right, HIDDEN);
    SPR_setDepth (spr_face_left, SPR_MIN_DEPTH+1); // Face background are above anything but faces
    SPR_setDepth (spr_face_right, SPR_MIN_DEPTH+1); // Face background are above anything but faces
}
