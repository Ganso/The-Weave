#include <genesis.h>
#include "globals.h"

void geesebumps_logo(void)
{
    Sprite *logo_text, *logo_lines1, *logo_lines2;

    initialize();

    VDP_setBackgroundColor(15);

    PAL_setPalette(PAL0, geesebumps_logo_bg.palette->data, DMA);
    PAL_setPalette(PAL1, geesebumps_logo_text.palette->data, DMA);
    PAL_setPalette(PAL2, geesebumps_logo_line1.palette->data, DMA);
    PAL_setPalette(PAL3, geesebumps_logo_line2.palette->data, DMA);

    VDP_drawImageEx(BG_A, &geesebumps_logo_bg, TILE_ATTR_FULL(PAL0, false, false, false, tile_ind), 0, 0, false, true);
    tile_ind+=geesebumps_logo_bg.tileset->numTile;

    logo_text = SPR_addSpriteSafe(&geesebumps_logo_text, 0, 0, TILE_ATTR(PAL1, false, false, false));
    logo_lines1 = SPR_addSpriteSafe(&geesebumps_logo_line1, 0, 0, TILE_ATTR(PAL2, false, false, false));
    logo_lines2 = SPR_addSpriteSafe(&geesebumps_logo_line2, 0, 0, TILE_ATTR(PAL3, false, false, false));
    
    SPR_update();
}