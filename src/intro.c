#include "globals.h"

/**
 * @brief Displays and handles the game's intro sequence
 * 
 * Shows the game logo with animated stars in the background, allows language selection,
 * and waits for player input to proceed. Handles all resource loading/unloading and
 * transitions including music and palette fades.
 */
void theweave_intro(void)
{
    Sprite *star[MAXSTARS];
    u8 nstar,x,y;
    bool exit_menu=false;
    
    initialize(false);

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
    play_music(music_intro);

    game_language=LANG_ENGLISH;    
    intro_update_language();
    while (!exit_menu) {
        for (nstar=0;nstar<MAXSTARS;nstar++) {
            if (SPR_isAnimationDone(star[nstar])) {
                x=random()%320;
                y=random()%224;
                SPR_setPosition(star[nstar],x,y);
                SPR_setAnimAndFrame(star[nstar],random()%3,0);
            }
        }
        exit_menu=intro_read_keys();
        SPR_update();
        SYS_doVBlankProcess();
    }

    // Fade out music and graphics
    fade_music(SCREEN_FPS*2);
    PAL_fadeOutAll(SCREEN_FPS*2, true);
    waitMs(2000);

    // Release everything
    SPR_reset();
    VDP_releaseAllSprites();
    VDP_clearPlane(BG_A, true);
    tile_ind-=intro_logo_bg.tileset->numTile;   
}

/**
 * @brief Updates the language selection display in the intro screen
 * 
 * Clears and redraws the language options (English/Spanish) text,
 * highlighting the currently selected language with brackets.
 */
void intro_update_language(void)
{
    char eng_text[40];
    char spa_text[40];
    
    VDP_clearTextLineBG(WINDOW,22);
    VDP_clearTextLineBG(WINDOW,24);

    if (game_language == LANG_ENGLISH) {
        strcpy(eng_text, "} English {");
        strcpy(spa_text, "Espa^ol");
    } else {
        strcpy(eng_text, "English");
        strcpy(spa_text, "} Espa^ol {");
    }

    VDP_drawTextBG(WINDOW, eng_text, (40 - strlen(eng_text)) >> 1, 22);
    VDP_drawTextBG(WINDOW, spa_text, (40 - strlen(spa_text)) >> 1, 24);
}

/**
 * @brief Handles player input during the intro sequence
 * 
 * Processes directional inputs to switch language selection and
 * A/Start buttons to confirm and exit the intro sequence.
 * 
 * @return true if player pressed A/Start to exit intro, false otherwise
 */
bool intro_read_keys(void)
{
    u16 joy_state;

    joy_state=JOY_readJoypad (JOY_ALL);

    switch (joy_state)
    {
        case BUTTON_LEFT:
        case BUTTON_RIGHT:
        case BUTTON_UP:
        case BUTTON_DOWN:
            while (JOY_readJoypad(JOY_ALL)!=0) SYS_doVBlankProcess();
            if (game_language==LANG_ENGLISH) game_language=LANG_SPANISH;
            else game_language=LANG_ENGLISH;
            intro_update_language();
            break;
        case BUTTON_A:
        case BUTTON_START:
            return true;    
        default:
            return false;
    }
    return false;
}
