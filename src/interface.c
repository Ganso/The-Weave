#include <genesis.h>
#include "globals.h"

// Global variable definitions
Sprite *spr_face_left;
Sprite *spr_face_right;
Sprite *spr_int_button_A;
Map *map_int_rod;
Sprite *spr_int_rod_1,*spr_int_rod_2,*spr_int_rod_3,*spr_int_rod_4,*spr_int_rod_5,*spr_int_rod_6;
Sprite *spr_int_enemy_rod_1,*spr_int_enemy_rod_2,*spr_int_enemy_rod_3,*spr_int_enemy_rod_4,*spr_int_enemy_rod_5,*spr_int_enemy_rod_6;
Sprite *spr_int_pentagram_1,*spr_int_pentagram_2,*spr_int_pentagram_3,*spr_int_pentagram_4,*spr_int_pentagram_5,*spr_int_pentagram_6;
Sprite *spr_int_life_counter;
Sprite *spr_pause_icon[5];
Sprite *spr_pattern_list_note[4];
bool interface_active;


// Show or hide the bottom interface of the game
void show_or_hide_interface(bool visible)
{
    if (interface_active==true) {
        if (visible == true) {
            // Draw the rod and pentagram images in the window plane
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
}

// Show or hide a specific note in the interface
void show_note(u8 nnote, bool visible)
{
    Sprite **pentsprite;
    Sprite **rodsprite;
    SpriteDefinition *pentsritedef;
    SpriteDefinition *rodspritedef;
    u8 *notesong;
    // u8 *notewav;
    u16 pent_x,rod_x;

    // Determine the appropriate sprites, sounds, and positions for the given note
    switch (nnote) 
    {
    case NOTE_MI:
        pentsprite=&spr_int_pentagram_1;
        rodsprite=&spr_int_rod_1;
        pentsritedef=(SpriteDefinition*) &int_pentagram_1_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_1_sprite;
        notesong=(u8*)snd_note_mi;
        pent_x=219;
        rod_x=24;
        break;
    case NOTE_FA:
        pentsprite=&spr_int_pentagram_2;
        rodsprite=&spr_int_rod_2;
        pentsritedef=(SpriteDefinition*) &int_pentagram_2_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_2_sprite;
        notesong=(u8*)snd_note_fa;
        pent_x=219+16;
        rod_x=24+32;
        break;
    case NOTE_SOL:
        pentsprite=&spr_int_pentagram_3;
        rodsprite=&spr_int_rod_3;
        pentsritedef=(SpriteDefinition*) &int_pentagram_3_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_3_sprite;
        notesong=(u8*)snd_note_sol;
        pent_x=219+32;
        rod_x=24+64;
        break;
    case NOTE_LA:
        pentsprite=&spr_int_pentagram_4;
        rodsprite=&spr_int_rod_4;
        pentsritedef=(SpriteDefinition*) &int_pentagram_4_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_4_sprite;
        notesong=(u8*)snd_note_la;
        pent_x=219+48;
        rod_x=24+96;
        break;
    case NOTE_SI:
        pentsprite=&spr_int_pentagram_5;
        rodsprite=&spr_int_rod_5;
        pentsritedef=(SpriteDefinition*) &int_pentagram_5_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_5_sprite;
        notesong=(u8*)snd_note_si;
        pent_x=219+64;
        rod_x=24+128;
        break;
    default: // NOTE_DO
        pentsprite=&spr_int_pentagram_6;
        rodsprite=&spr_int_rod_6;
        pentsritedef=(SpriteDefinition*) &int_pentagram_6_sprite;
        rodspritedef=(SpriteDefinition*) &int_rod_6_sprite;
        notesong=(u8*)snd_note_do;
        pent_x=219+80;
        rod_x=24+160;
        break;
    }

    if (visible == true) {
        // Add rod and pentagram sprites if they don't exist
        if (*rodsprite==NULL) *rodsprite = SPR_addSpriteSafe (rodspritedef, rod_x, 212, TILE_ATTR(PAL2, false, false, false));
        if (*pentsprite==NULL && player_patterns_enabled) *pentsprite = SPR_addSpriteSafe (pentsritedef, pent_x, 180, TILE_ATTR(PAL2, false, false, false));
        play_music(notesong); // Play the note sound
    }
    else {
        // Release both pentagram and rod sprites if they exist
        if (*rodsprite!=NULL) {
            SPR_releaseSprite(*rodsprite);
            *rodsprite=NULL;
        if (*pentsprite!=NULL && player_patterns_enabled) {
            SPR_releaseSprite(*pentsprite);
            *pentsprite=NULL;
        }
        }
    }
    SPR_update();
}

// Hide all rod icons in the interface
void hide_rod_icons(void)
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

// Hide all pentagram icons in the interface
void hide_pentagram_icons(void)
{
    // Release all pentagram sprites
    if (spr_int_pentagram_1!=NULL) SPR_releaseSprite(spr_int_pentagram_1);
    if (spr_int_pentagram_2!=NULL) SPR_releaseSprite(spr_int_pentagram_2);
    if (spr_int_pentagram_3!=NULL) SPR_releaseSprite(spr_int_pentagram_3);
    if (spr_int_pentagram_4!=NULL) SPR_releaseSprite(spr_int_pentagram_4);
    if (spr_int_pentagram_5!=NULL) SPR_releaseSprite(spr_int_pentagram_5);
    if (spr_int_pentagram_6!=NULL) SPR_releaseSprite(spr_int_pentagram_6);
    SPR_update();
}

// Hide all pattern icons in the interface
void hide_pattern_icons(void)
{
    u16 npattern;

    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        if (obj_pattern[npattern].sd!=NULL) {
            SPR_releaseSprite(obj_pattern[npattern].sd);
            obj_pattern[npattern].sd=NULL;
        }
    }
}

// Show or hide a specific pattern icon in the interface
void show_pattern_icon(u16 npattern, bool show, bool priority)
{
    u8 npal = PAL2;
    const SpriteDefinition *nsprite = NULL;

    if (show==TRUE) {
        // Select the appropriate sprite based on the pattern
        if (npattern==PTRN_ELECTRIC) nsprite = &int_pattern_thunder;
        if (npattern==PTRN_HIDE) nsprite = &int_pattern_hide;
        if (npattern==PTRN_OPEN) nsprite = &int_pattern_open;
        if (npattern==PTRN_SLEEP) nsprite = &int_pattern_sleep;
        
        // Add the sprite if it doesn't exist
        if (obj_pattern[npattern].sd==NULL) obj_pattern[npattern].sd = SPR_addSpriteSafe(nsprite, SCREEN_WIDTH-40, 4, TILE_ATTR(npal, priority, false, false));
        SPR_setAlwaysOnTop(obj_pattern[npattern].sd);
    }
    else {
        // Release the sprite if it exists
        SPR_releaseSprite(obj_pattern[npattern].sd);
        obj_pattern[npattern].sd=NULL;
    }
}

// Function to hide all sprites and save their state
SpriteState* hideAllSprites(u16* count) {
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

// Function to restore the visibility of sprites
void restoreSpritesVisibility(SpriteState* states, u16 count) {
    for (u16 i = 0; i < count; i++) {
        if (states[i].sprite != NULL) {
            SPR_setVisibility(states[i].sprite, states[i].visibility);
        }
    }
    
    // Free allocated memory
    MEM_free(states);
}

// Display the pause/state screen
void pause_screen(void) {
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
    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        if (obj_pattern[npattern].active==true) {
            #ifdef DEBUG_ON
             kprintf("Patron %d activo", npattern);
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
                    if (selected_pattern==MAX_PATTERNS) selected_pattern=0;
                    if (obj_pattern[selected_pattern].active==true) next_pattern_found=true;
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
                    if (selected_pattern==255) selected_pattern=MAX_PATTERNS-1;
                    if (obj_pattern[selected_pattern].active==true) next_pattern_found=true;
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

// Show or hide the pattern list in the pause screen
void show_pause_pattern_list(bool show, u8 active_pattern)
{
    u16 x_initial,x, nicon;
    u8 nnote, npattern, num_active_patterns=0;
    bool priority;

    for (npattern=0;npattern<MAX_PATTERNS;npattern++) // Count active patterns
        if (obj_pattern[npattern].active==true) num_active_patterns++;

    x_initial = (134 - 24 * num_active_patterns); // Calculate initial X position
    x = x_initial;
    nicon = 0;

    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        if (obj_pattern[npattern].active==true) {
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

// Show one of the notes of a pattern in the pattern list (Pause screen)
void show_note_in_pause_pattern_list(u8 npattern, u8 nnote, bool show)
{
    SpriteDefinition *pentsprite;
    u8 note;
    u16 x;

    note=obj_pattern[npattern].notes[nnote]; // Get the note at this position in the pattern

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

// Show or hide the icon of a pattern spell in the pause list
void show_icon_in_pause_list(u16 npattern, u8 nicon, u16 x, bool show, bool priority)
{
    u8 npal = PAL2;
    const SpriteDefinition *nsprite = NULL;

    if (show==TRUE) {
        // Select the appropriate sprite based on the pattern
        if (npattern==PTRN_ELECTRIC) nsprite = &int_pattern_thunder;
        if (npattern==PTRN_HIDE) nsprite = &int_pattern_hide;
        if (npattern==PTRN_OPEN) nsprite = &int_pattern_open;
        if (npattern==PTRN_SLEEP) nsprite = &int_pattern_sleep;

        if (spr_pause_icon[nicon]==NULL) spr_pause_icon[nicon] = SPR_addSpriteSafe(nsprite, x, 182, TILE_ATTR(npal, priority, false, false));
    }
    else {
        if (spr_pause_icon[nicon] != NULL) {
            SPR_releaseSprite(spr_pause_icon[nicon]);
            spr_pause_icon[nicon]=NULL;
        }
    }
}
