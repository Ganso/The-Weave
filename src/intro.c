#include <genesis.h>
#include "globals.h"

void theweave_intro(void)
{
    Sprite *star[20];
    u8 nstar,x,y;

    initialize();

    PAL_setPalette(PAL0, intro_logo_bg.palette->data, DMA);
    PAL_setPalette(PAL1, intro_stars_sprite.palette->data, DMA);

    // Initialize stars
    for (nstar=0;nstar<20;nstar++) {
        x=random()%320;
        y=random()%224;
        star[nstar]=SPR_addSpriteSafe(&intro_stars_sprite, x, y, TILE_ATTR(PAL1, false, false, false));
        if (random()%2) SPR_setAnim(star[nstar],1);
    }

    // Game Logo
    VDP_drawImageEx(BG_A, &intro_logo_bg, TILE_ATTR_FULL(PAL0, true, false, false, tile_ind), 0, 0, false, true);
    tile_ind+=intro_logo_bg.tileset->numTile;

    // Background music
    XGM2_play(music_intro);

    while (true) {
        if ((random()%10)==0) {
            nstar=random()%20;
            x=random()%320;
            y=random()%224;
            SPR_setPosition(star[nstar],x,y);
        }
        SPR_update();
        SYS_doVBlankProcess();
    }
}