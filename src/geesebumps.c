#include <genesis.h>
#include "globals.h"

void geesebumps_logo(void)
{
    Sprite *logo_text, *logo_lines1, *logo_lines2;

    VDP_setBackgroundColor(13);

    PAL_setPalette(PAL0, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL1, geesebumps_pal_white.data, DMA);
    PAL_setPalette(PAL2, geesebumps_pal_white.data, DMA);
    PAL_setPalette(PAL3, geesebumps_pal_white.data, DMA);

    // Background music
    XGM2_play(music_geesebumps);
    
    // Fist part of the logo (Goose)
    VDP_drawImageEx(BG_A, &geesebumps_logo_bg, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind), 0, 0, false, true);
    tile_ind+=geesebumps_logo_bg.tileset->numTile;

    // Load every other srpite
    logo_text = SPR_addSpriteSafe(&geesebumps_logo_text, 60, 163, TILE_ATTR(PAL1, false, false, false));
    logo_lines1 = SPR_addSpriteSafe(&geesebumps_logo_line1, 81-180, 55, TILE_ATTR(PAL2, false, false, false));
    logo_lines2 = SPR_addSpriteSafe(&geesebumps_logo_line2, 81-180, 84, TILE_ATTR(PAL3, false, false, false));
    SPR_setVisibility(logo_text, HIDDEN);
    SPR_update();

    // Second part (Text) fade in
    PAL_fade(0, 15, geesebumps_pal_black.data, geesebumps_logo_bg.palette->data, SCREEN_FPS*2, false);
    SPR_setVisibility(logo_text, VISIBLE);
    SPR_update();
    PAL_fade(16, 31, geesebumps_pal_white.data, geesebumps_logo_text.palette->data, SCREEN_FPS*2, false);

    // Third and fourth part (Color lines) fade in and scroll
    PAL_initFade(32, 63, geesebumps_pal_white2.data, geesebumps_pal_lines.data, SCREEN_FPS*3);
    for (u16 difx=180; difx>0; difx--) {
        SPR_setPosition(logo_lines1, 81-difx, 55);
        SPR_setPosition(logo_lines2, 81-difx, 84);
        SPR_update();
        PAL_doFadeStep();
        SYS_doVBlankProcess();
    }

    // Pause and fade out
    waitMs(4000);
    PAL_fadeOutAll(SCREEN_FPS*2, false);

    // Release everything
    VDP_releaseAllSprites();
    VDP_clearPlane(BG_A, true);
    tile_ind-=geesebumps_logo_bg.tileset->numTile;   
}