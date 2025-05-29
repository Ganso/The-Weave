#include "globals.h"

Sprite *spr_face_left;                    // Left character face background sprite
Sprite *spr_face_right;                   // Right character face background sprite
Sprite *spr_int_button_A;                 // Button A interface sprite
Map *map_int_rod;                         // Rod interface map
Sprite *spr_int_rod_1,*spr_int_rod_2,*spr_int_rod_3,*spr_int_rod_4,*spr_int_rod_5,*spr_int_rod_6;    // Player rod note sprites
Sprite *spr_int_enemy_rod_1,*spr_int_enemy_rod_2,*spr_int_enemy_rod_3,*spr_int_enemy_rod_4,*spr_int_enemy_rod_5,*spr_int_enemy_rod_6;    // Enemy rod note sprites
Sprite *spr_int_pentagram_1,*spr_int_pentagram_2,*spr_int_pentagram_3,*spr_int_pentagram_4,*spr_int_pentagram_5,*spr_int_pentagram_6;    // Pentagram note sprites
Sprite *spr_int_life_counter;             // Life counter interface sprite
Sprite *spr_pause_icon[5];                // Pattern icons shown in pause screen
Sprite *spr_pattern_list_note[4];         // Note sprites shown in pause pattern list
bool interface_active;                     // Whether game interface is currently shown


void show_or_hide_interface(bool visible)    // Toggle visibility of game's bottom interface
{
    if (!interface_active) return; // If interface is not active, do nothing

    if (visible == true) {
        // Draw the rod and pentagram images in the window plane
        VDP_drawImageEx(WINDOW, &int_screen_limit, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 0, 22, false, true);
        tile_ind+=int_screen_limit.tileset->numTile;
        VDP_drawImageEx(WINDOW, &int_rod_image, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 0, 23, false, true);
        tile_ind+=int_rod_image.tileset->numTile;
        VDP_drawImageEx(WINDOW, &int_pentagram_image, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 27, 22, false, true);
        tile_ind+=int_pentagram_image.tileset->numTile;
    } 
    else {
        // Clear the window plane and hide rod and pentagram icons
        tile_ind-=int_rod_image.tileset->numTile;
        tile_ind-=int_pentagram_image.tileset->numTile;
        VDP_clearPlane(WINDOW, true);
        hide_rod_icons();
        hide_pentagram_icons();
    }
}

// Display or hide one musical note (rod + pentagram)
void show_note(u8 nnote, bool visible)
{
    // Map each note-code to its sprite pointers / definitions / X offsets
    Sprite **pentSpr, **rodSpr;
    const SpriteDefinition *pentDef, *rodDef;
    u16 pentX, rodX;

    switch (nnote)
    {
        case NOTE_MI:  pentSpr=&spr_int_pentagram_1; rodSpr=&spr_int_rod_1;
                       pentDef=&int_pentagram_1_sprite; rodDef=&int_rod_1_sprite;
                       pentX=219; rodX=24;  break;
        case NOTE_FA:  pentSpr=&spr_int_pentagram_2; rodSpr=&spr_int_rod_2;
                       pentDef=&int_pentagram_2_sprite; rodDef=&int_rod_2_sprite;
                       pentX=235; rodX=56;  break;
        case NOTE_SOL: pentSpr=&spr_int_pentagram_3; rodSpr=&spr_int_rod_3;
                       pentDef=&int_pentagram_3_sprite; rodDef=&int_rod_3_sprite;
                       pentX=251; rodX=88;  break;
        case NOTE_LA:  pentSpr=&spr_int_pentagram_4; rodSpr=&spr_int_rod_4;
                       pentDef=&int_pentagram_4_sprite; rodDef=&int_rod_4_sprite;
                       pentX=267; rodX=120; break;
        case NOTE_SI:  pentSpr=&spr_int_pentagram_5; rodSpr=&spr_int_rod_5;
                       pentDef=&int_pentagram_5_sprite; rodDef=&int_rod_5_sprite;
                       pentX=283; rodX=152; break;
        default:       pentSpr=&spr_int_pentagram_6; rodSpr=&spr_int_rod_6;
                       pentDef=&int_pentagram_6_sprite; rodDef=&int_rod_6_sprite;
                       pentX=299; rodX=184; break;
    }

    if (visible) {
        // Create sprite if it does not yet exist
        if (*rodSpr == NULL)
            *rodSpr = SPR_addSpriteSafe(rodDef, rodX, 212,
                                        TILE_ATTR(PAL2,false,false,false));
        if (*pentSpr == NULL && player_patterns_enabled)
            *pentSpr = SPR_addSpriteSafe(pentDef, pentX, 180,
                                        TILE_ATTR(PAL2,false,false,false));
    } else {
        // Destroy sprite if present
        if (*rodSpr)  { SPR_releaseSprite(*rodSpr);  *rodSpr = NULL; }
        if (*pentSpr) { SPR_releaseSprite(*pentSpr); *pentSpr = NULL; }
    }

    // Leave the global engine (next_frame) to call SPR_update().
    // Doing it here is safe but costs CPU if you call show_note()
    // several times in a single frame. Remove if you prefer.
    SPR_update();
}

void hide_rod_icons(void)    // Remove all rod note sprites from interface
{
    // Release all rod sprites and set pointers to NULL
    if (spr_int_rod_1!=NULL) SPR_releaseSprite(spr_int_rod_1);
    if (spr_int_rod_2!=NULL) SPR_releaseSprite(spr_int_rod_2);
    if (spr_int_rod_3!=NULL) SPR_releaseSprite(spr_int_rod_3);
    if (spr_int_rod_4!=NULL) SPR_releaseSprite(spr_int_rod_4);
    if (spr_int_rod_5!=NULL) SPR_releaseSprite(spr_int_rod_5);
    if (spr_int_rod_6!=NULL) SPR_releaseSprite(spr_int_rod_6);
    spr_int_rod_1=NULL;
    spr_int_rod_2=NULL;
    spr_int_rod_3=NULL;
    spr_int_rod_4=NULL;
    spr_int_rod_5=NULL;
    spr_int_rod_6=NULL;
    SPR_update();
}

void hide_pentagram_icons(void)    // Remove all pentagram note sprites from interface
{
    // Release all pentagram sprites and set pointers to NULL
    if (spr_int_pentagram_1!=NULL) SPR_releaseSprite(spr_int_pentagram_1);
    if (spr_int_pentagram_2!=NULL) SPR_releaseSprite(spr_int_pentagram_2);
    if (spr_int_pentagram_3!=NULL) SPR_releaseSprite(spr_int_pentagram_3);
    if (spr_int_pentagram_4!=NULL) SPR_releaseSprite(spr_int_pentagram_4);
    if (spr_int_pentagram_5!=NULL) SPR_releaseSprite(spr_int_pentagram_5);
    if (spr_int_pentagram_6!=NULL) SPR_releaseSprite(spr_int_pentagram_6);
    spr_int_pentagram_1=NULL;
    spr_int_pentagram_2=NULL;
    spr_int_pentagram_3=NULL;
    spr_int_pentagram_4=NULL;
    spr_int_pentagram_5=NULL;
    spr_int_pentagram_6=NULL;
    SPR_update();
}

void hide_pattern_icons(void)    // Remove all pattern spell icons from interface
{
    u16 npattern;

    for (npattern=0;npattern<MAX_PLAYER_PATTERNS;npattern++) {
    if (playerPatterns[npattern].icon!=NULL) {
            SPR_releaseSprite(playerPatterns[npattern].icon);
            playerPatterns[npattern].icon=NULL;
        }
    }
}

void show_pattern_icon(u16 npattern, bool show, bool priority)    // Display or hide a pattern spell icon in interface
{
    u8 npal = PAL2;
    const SpriteDefinition *nsprite = NULL;

    if (show==TRUE) {
        // Select the appropriate sprite based on the pattern
        if (npattern==PATTERN_THUNDER) nsprite = &int_pattern_thunder;
        if (npattern==PATTERN_HIDE) nsprite = &int_pattern_hide;
        if (npattern==PATTERN_OPEN) nsprite = &int_pattern_open;
        if (npattern==PATTERN_SLEEP) nsprite = &int_pattern_sleep;
        
        // Add the sprite if it doesn't exist
        if (playerPatterns[npattern].icon==NULL) playerPatterns[npattern].icon = SPR_addSpriteSafe(nsprite, SCREEN_WIDTH-40, 4, TILE_ATTR(npal, priority, false, false));
        SPR_setAlwaysOnTop(playerPatterns[npattern].icon);
    }
    else {
        // Release the sprite if it exists
        SPR_releaseSprite(playerPatterns[npattern].icon);
        playerPatterns[npattern].icon=NULL;
    }
}

SpriteState* hideAllSprites(u16* count)    // Hide all active sprites and save their visibility state
{
    Sprite* currentSprite = firstSprite;
    u16 spriteCount = 0;
    
    // First, count how many sprites there are
    while (currentSprite != NULL) {
        spriteCount++;
        currentSprite = currentSprite->next;
    }
    
    // Allocate memory for the state array
    SpriteState* states = MEM_alloc(spriteCount * sizeof(SpriteState));
    
    // Return to the first sprite
    currentSprite = firstSprite;
    u16 index = 0;
    
    // Traverse again, saving the state and hiding
    while (currentSprite != NULL) {
        states[index].sprite = currentSprite;
        states[index].visibility = SPR_getVisibility(currentSprite);
        
        // Hide the sprite
        SPR_setVisibility(currentSprite, HIDDEN);
        
        currentSprite = currentSprite->next;
        index++;
    }
    
    *count = spriteCount;
    return states;
}

void restoreSpritesVisibility(SpriteState* states, u16 count)    // Restore previously saved sprite visibility states
{
    for (u16 i = 0; i < count; i++) {
        if (states[i].sprite != NULL) {
            SPR_setVisibility(states[i].sprite, states[i].visibility);
        }
    }
    
    // Free allocated memory
    MEM_free(states);
}

void pause_screen(void)    // Handle pause screen with pattern spell selection
{
    u16 value; // Joypad value
    u8 old_pattern,selected_pattern,npattern,num_active_patterns=0;
    bool next_pattern_found;

    u16 spriteCount;
    SpriteState* savedStates;

    // Initialize pattern list note sprites to NULL
    for (u8 i = 0; i < 4; i++) {
        spr_pattern_list_note[i] = NULL;
    }

    VDP_setHilightShadow(true); // Dim screen
    show_or_hide_interface(false); // Hide interface
    //show_or_hide_enemy_combat_interface(false); // Hide combat interface
    savedStates = hideAllSprites(&spriteCount); // Hide every sprite and save state

    // Find the first active pattern
    selected_pattern=254;
    for (npattern=0;npattern<MAX_PLAYER_PATTERNS;npattern++) {
        if (playerPatterns[npattern].enabled==true) {
            #ifdef DEBUG_ON
             dprintf(2,"Patron %d activo", npattern);
            #endif
            num_active_patterns++;
            if (selected_pattern==254) selected_pattern=npattern;
        } 
    }

    if (num_active_patterns!=0) show_pause_pattern_list(true,selected_pattern);

    value = JOY_readJoypad(JOY_ALL);
    while ( (value & BUTTON_START) == 0 ) { // Wait until START button is pressed again
        value = JOY_readJoypad(JOY_ALL);
        if (num_active_patterns>1) { // Only allow pattern switching if there's more than one active pattern
            if (value & BUTTON_RIGHT) { // Find next active pattern
                next_pattern_found=false;
                old_pattern=selected_pattern;
                selected_pattern++;
                while (next_pattern_found==false)
                {
                    if (selected_pattern==MAX_PLAYER_PATTERNS) selected_pattern=0;
                    if (playerPatterns[selected_pattern].enabled==true) next_pattern_found=true;
                    else selected_pattern++;
                }
                show_pause_pattern_list(false,old_pattern); // Hide previous pattern
                show_pause_pattern_list(true,selected_pattern); // Show new pattern
            }
            if (value & BUTTON_LEFT) { // Find previous active pattern
                next_pattern_found=false;
                old_pattern=selected_pattern;
                selected_pattern--;
                while (next_pattern_found==false)
                {
                    if (selected_pattern==255) selected_pattern=MAX_PLAYER_PATTERNS-1;
                    if (playerPatterns[selected_pattern].enabled==true) next_pattern_found=true;
                    else selected_pattern--;
                }            
                show_pause_pattern_list(false,old_pattern); // Hide previous pattern
                show_pause_pattern_list(true,selected_pattern); // Show new pattern
            }
            while ((value & BUTTON_LEFT) || (value & BUTTON_RIGHT)) // Wait until LEFT or RIGHT is released
            {
                value = JOY_readJoypad(JOY_ALL);
                SYS_doVBlankProcess();
            }
        }
        SPR_update();
        SYS_doVBlankProcess();
    }
    while ( value & BUTTON_START ) { // Now wait until START is released
        value = JOY_readJoypad(JOY_ALL);
        SYS_doVBlankProcess();
    }

    if (num_active_patterns != 0) {
        show_pause_pattern_list(false, selected_pattern); // Hide last selected pattern
    }

    // Release any remaining pattern list note sprites
    for (u8 nnote = 0; nnote < 4; nnote++) {
        if (spr_pattern_list_note[nnote] != NULL) {
            SPR_releaseSprite(spr_pattern_list_note[nnote]);
            spr_pattern_list_note[nnote] = NULL;
        }
    }

    show_or_hide_interface(true); // Show interface again
    //show_or_hide_enemy_combat_interface(true); // Show combat interface again
    restoreSpritesVisibility(savedStates, spriteCount); // Restore sprites visibility
    VDP_setHilightShadow(false); // Relit screen
    SPR_update();
    VDP_waitVSync();
}

void show_pause_pattern_list(bool show, u8 active_pattern)    // Display pattern list in pause screen with active pattern highlighted
{
    u16 x_initial,x, nicon;
    u8 nnote, npattern, num_active_patterns=0;
    bool priority;

    for (npattern=0;npattern<MAX_PLAYER_PATTERNS;npattern++) // Count active patterns
        if (playerPatterns[npattern].enabled==true) num_active_patterns++;

    x_initial = (134 - 24 * num_active_patterns); // Calculate initial X position
    x = x_initial;
    nicon = 0;

    for (npattern=0;npattern<MAX_PLAYER_PATTERNS;npattern++) {
        if (playerPatterns[npattern].enabled==true) {
            priority = (npattern==active_pattern); // Highlight active pattern
            show_icon_in_pause_list(npattern, nicon, x, show, priority); // Show or hide pattern icon
            x+=48;
            nicon++;
        }
    }

    for (nnote=0;nnote<4;nnote++) {
        show_note_in_pause_pattern_list(active_pattern,nnote,show); // Show notes for the active pattern
    }
}

void show_note_in_pause_pattern_list(u8 npattern, u8 nnote, bool show)    // Display a note in the pause screen pattern list
{
    SpriteDefinition *pentsprite;
    u8 note;
    u16 x;

    note=playerPatterns[npattern].notes[nnote]; // Get the note for this pattern 

    // Select the appropriate pentagram sprite based on the note
    switch (note) 
    {
    case NOTE_MI:
        pentsprite=(SpriteDefinition*) &int_pentagram_1_sprite;
        break;
    case NOTE_FA:
        pentsprite=(SpriteDefinition*) &int_pentagram_2_sprite;
        break;
    case NOTE_SOL:
        pentsprite=(SpriteDefinition*) &int_pentagram_3_sprite;
        break;
    case NOTE_LA:
        pentsprite=(SpriteDefinition*) &int_pentagram_4_sprite;
        break;
    case NOTE_SI:
        pentsprite=(SpriteDefinition*) &int_pentagram_5_sprite;
        break;
    default:
        pentsprite=(SpriteDefinition*) &int_pentagram_6_sprite;
        break;
    }

    if (show==true) {
        x=251+nnote*16;
        spr_pattern_list_note[nnote]=SPR_addSpriteSafe(pentsprite, x, 180, TILE_ATTR(PAL2,false,false,false));
    }
    else {
        if (spr_pattern_list_note[nnote] != NULL) {
            SPR_releaseSprite(spr_pattern_list_note[nnote]);
            spr_pattern_list_note[nnote] = NULL;
        }
    }
}

void show_icon_in_pause_list(u16 npattern, u8 nicon, u16 x, bool show, bool priority)    // Display a pattern icon in the pause screen list
{
    u8 npal = PAL2;
    const SpriteDefinition *nsprite = NULL;

    if (show==TRUE) {
        // Select the appropriate sprite based on the pattern
        if (npattern==PATTERN_THUNDER) nsprite = &int_pattern_thunder;
        if (npattern==PATTERN_HIDE) nsprite = &int_pattern_hide;
        if (npattern==PATTERN_OPEN) nsprite = &int_pattern_open;
        if (npattern==PATTERN_SLEEP) nsprite = &int_pattern_sleep;

        if (spr_pause_icon[nicon]==NULL) spr_pause_icon[nicon] = SPR_addSpriteSafe(nsprite, x, 182, TILE_ATTR(npal, priority, false, false));
    }
    else {
        if (spr_pause_icon[nicon] != NULL) {
            SPR_releaseSprite(spr_pause_icon[nicon]);
            spr_pause_icon[nicon]=NULL;
        }
    }
}

// Check the status of the current pattern, including note playing and expiration
void check_pattern_status(void)
{
    // Count frames since the last player note
    ++combatContext.noteTimer;

    // Abort if the player stopped in the middle of a pattern
    if (combatContext.playerNotes &&
        combatContext.noteTimer > calc_ticks(MAX_PATTERN_WAIT_TIME))
    {
        dprintf(2,"Abort: playerNotes=%u  noteTimer=%u", combatContext.playerNotes, combatContext.noteTimer);

        reset_note_queue();                         // hides all four icons
        obj_character[active_character].state = STATE_IDLE;
        play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
    }
}
