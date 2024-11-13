#include <genesis.h>
#include "globals.h"

void theweave_intro(void)
{
    Sprite *star[MAXSTARS];
    u8 nstar,x,y;

    initialize();
    
    PAL_setPalette(PAL0, geesebumps_pal_black.data, DMA);
    PAL_setPalette(PAL1, geesebumps_pal_black.data, DMA);

    // Initialize stars
    for (nstar=0;nstar<MAXSTARS;nstar++) {
        x=random()%320;
        y=random()%224;
        star[nstar]=SPR_addSpriteSafe(&intro_stars_sprite, x, y, TILE_ATTR(PAL1, false, false, false));
        SPR_setAnimAndFrame(star[nstar],random()%3,random()%8);
    }

    // Game Logo
    VDP_drawImageEx(BG_A, &intro_logo_bg, TILE_ATTR_FULL(PAL0, true, false, false, tile_ind), 0, 0, false, true);
    tile_ind+=intro_logo_bg.tileset->numTile;

    // Version number
    VDP_drawTextBG(BG_A, GAMEVERSION, 40-strlen(GAMEVERSION), 0);

    // Fade from black
    PAL_fade(0, 15, geesebumps_pal_black.data, intro_logo_bg.palette->data, SCREEN_FPS*2, false);
    PAL_fade(16, 31, geesebumps_pal_black.data, intro_stars_sprite.palette->data, SCREEN_FPS/2, false);

    // Disable star loops
    for (nstar=0;nstar<MAXSTARS;nstar++) SPR_setAnimationLoop(star[nstar],false);

    // Background music
    XGM2_play(music_intro);

    while (true) {
        for (nstar=0;nstar<MAXSTARS;nstar++) {
            if (SPR_getAnimationDone(star[nstar])) {
                x=random()%320;
                y=random()%224;
                SPR_setPosition(star[nstar],x,y);
                SPR_setAnimAndFrame(star[nstar],random()%3,0);
            }
        }
        SPR_update();
        SYS_doVBlankProcess();
    }
}