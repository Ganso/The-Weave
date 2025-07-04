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
bool interface_active;                    // Whether game interface is currently shown
static u16 interface_sprite_count;        // How many interface sprites were hidden
bool player_has_paused;                   // Has the player paused the game since last reset?

// Struct to store the animation state of a sprite
typedef struct {
    s16 animInd;
    s16 frameInd;
    bool autoAnim;
} AnimState;

static AnimState saved_chr_anim[MAX_CHR];     // Saved animations for characters
static AnimState saved_enemy_anim[MAX_ENEMIES]; // Saved animations for enemies

// Save current animation state for all characters and enemies and set them to IDLE
static void save_animation_states(void)
{
    for (u16 i = 0; i < MAX_CHR; i++)
    {
        saved_chr_anim[i].animInd  = 0;
        saved_chr_anim[i].frameInd = 0;
        saved_chr_anim[i].autoAnim = false;

        if (!obj_character[i].active || spr_chr[i] == NULL) continue;

        saved_chr_anim[i].animInd  = spr_chr[i]->animInd;
        saved_chr_anim[i].frameInd = spr_chr[i]->frameInd;
        saved_chr_anim[i].autoAnim = SPR_getAutoAnimation(spr_chr[i]);

        SPR_setAutoAnimation(spr_chr[i], FALSE);
        SPR_setAnimAndFrame(spr_chr[i], ANIM_IDLE, 0);
    }

    for (u16 e = 0; e < MAX_ENEMIES; e++)
    {
        saved_enemy_anim[e].animInd  = 0;
        saved_enemy_anim[e].frameInd = 0;
        saved_enemy_anim[e].autoAnim = false;

        if (!obj_enemy[e].obj_character.active || spr_enemy[e] == NULL) continue;

        saved_enemy_anim[e].animInd  = spr_enemy[e]->animInd;
        saved_enemy_anim[e].frameInd = spr_enemy[e]->frameInd;
        saved_enemy_anim[e].autoAnim = SPR_getAutoAnimation(spr_enemy[e]);

        SPR_setAutoAnimation(spr_enemy[e], FALSE);
        SPR_setAnimAndFrame(spr_enemy[e], ANIM_IDLE, 0);
    }
}

// Restore animations saved with save_animation_states()
static void restore_animation_states(void)
{
    for (u16 i = 0; i < MAX_CHR; i++)
    {
        if (!obj_character[i].active || spr_chr[i] == NULL) continue;

        SPR_setAnimAndFrame(spr_chr[i], saved_chr_anim[i].animInd, saved_chr_anim[i].frameInd);
        SPR_setAutoAnimation(spr_chr[i], saved_chr_anim[i].autoAnim);
        obj_character[i].animation = saved_chr_anim[i].animInd;
    }

    for (u16 e = 0; e < MAX_ENEMIES; e++)
    {
        if (!obj_enemy[e].obj_character.active || spr_enemy[e] == NULL) continue;

        SPR_setAnimAndFrame(spr_enemy[e], saved_enemy_anim[e].animInd, saved_enemy_anim[e].frameInd);
        SPR_setAutoAnimation(spr_enemy[e], saved_enemy_anim[e].autoAnim);
        obj_enemy[e].obj_character.animation = saved_enemy_anim[e].animInd;
    }
}


void show_or_hide_interface(bool visible)    // Toggle visibility of game's bottom interface
{
    if (!interface_active) return; // If interface is not active, do nothing

    if (visible == true) {
        // Draw the rod and pentagram images in the window plane
        VDP_drawImageEx(WINDOW, &int_screen_limit, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 0, 22, false, true);
        tile_ind+=int_screen_limit.tileset->numTile;
        VDP_drawImageEx(WINDOW, &int_rod_image, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 0, 24, false, true);
        tile_ind+=int_rod_image.tileset->numTile;
        VDP_drawImageEx(WINDOW, &int_pentagram_image, TILE_ATTR_FULL(PAL2, false, false, false, tile_ind), 27, 22, false, true);
        tile_ind+=int_pentagram_image.tileset->numTile;
    } 
    else {
        // Clear the window plane and hide rod and pentagram icons
        tile_ind-=int_rod_image.tileset->numTile;
        tile_ind-=int_pentagram_image.tileset->numTile;
        tile_ind-=int_screen_limit.tileset->numTile;
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
            *rodSpr = SPR_addSpriteSafe(rodDef, rodX, 210,
                                        TILE_ATTR(PAL2,false,false,false));
        if (*pentSpr == NULL && player_patterns_enabled)
            *pentSpr = SPR_addSpriteSafe(pentDef, pentX, 178,
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

SpriteState* hide_all_sprites(u16* count)    // Hide all active sprites and save their visibility state
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

SpriteState* hide_interface_sprites(void)    // Hide HUD/interface sprites only
{
    u16 index = 0;
    const u16 spriteCount = 1 + 6 + 6 + 6 + 1 + MAX_PLAYER_PATTERNS; // button + rods + enemy rods + pents + life + icons
    SpriteState* states = MEM_alloc(spriteCount * sizeof(SpriteState));

    // Button A
    states[index].sprite = spr_int_button_A;
    states[index].visibility = spr_int_button_A ? SPR_getVisibility(spr_int_button_A) : HIDDEN;
    if (spr_int_button_A) SPR_setVisibility(spr_int_button_A, HIDDEN);
    index++;

    // Player rod notes
    Sprite* rods[6] = {spr_int_rod_1, spr_int_rod_2, spr_int_rod_3, spr_int_rod_4, spr_int_rod_5, spr_int_rod_6};
    for (u16 i=0; i<6; i++) {
        states[index].sprite = rods[i];
        states[index].visibility = rods[i] ? SPR_getVisibility(rods[i]) : HIDDEN;
        if (rods[i]) SPR_setVisibility(rods[i], HIDDEN);
        index++;
    }

    // Enemy rod notes
    Sprite* enemy_rods[6] = {spr_int_enemy_rod_1, spr_int_enemy_rod_2, spr_int_enemy_rod_3, spr_int_enemy_rod_4, spr_int_enemy_rod_5, spr_int_enemy_rod_6};
    for (u16 i=0; i<6; i++) {
        states[index].sprite = enemy_rods[i];
        states[index].visibility = enemy_rods[i] ? SPR_getVisibility(enemy_rods[i]) : HIDDEN;
        if (enemy_rods[i]) SPR_setVisibility(enemy_rods[i], HIDDEN);
        index++;
    }

    // Pentagram notes
    Sprite* pents[6] = {spr_int_pentagram_1, spr_int_pentagram_2, spr_int_pentagram_3, spr_int_pentagram_4, spr_int_pentagram_5, spr_int_pentagram_6};
    for (u16 i=0; i<6; i++) {
        states[index].sprite = pents[i];
        states[index].visibility = pents[i] ? SPR_getVisibility(pents[i]) : HIDDEN;
        if (pents[i]) SPR_setVisibility(pents[i], HIDDEN);
        index++;
    }

    // Life counter
    states[index].sprite = spr_int_life_counter;
    states[index].visibility = spr_int_life_counter ? SPR_getVisibility(spr_int_life_counter) : HIDDEN;
    if (spr_int_life_counter) SPR_setVisibility(spr_int_life_counter, HIDDEN);
    index++;

    // Pattern icons
    for (u16 p=0; p<MAX_PLAYER_PATTERNS; p++) {
        states[index].sprite = playerPatterns[p].icon;
        states[index].visibility = playerPatterns[p].icon ? SPR_getVisibility(playerPatterns[p].icon) : HIDDEN;
        if (playerPatterns[p].icon) SPR_setVisibility(playerPatterns[p].icon, HIDDEN);
        index++;
    }

    interface_sprite_count = index;
    return states;
}

void show_interface_sprites(SpriteState* states)    // Restore HUD/interface sprites
{
    for (u16 i = 0; i < interface_sprite_count; i++)
    {
        if (states[i].sprite != NULL)
        {
            SPR_setVisibility(states[i].sprite, states[i].visibility);
        }
    }

    MEM_free(states);
}

void restore_sprites_visibility(SpriteState* states, u16 count)    // Restore previously saved sprite visibility states
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

    SpriteState* savedStates;

    // Initialize pattern list note sprites to NULL
    for (u8 i = 0; i < 4; i++) {
        spr_pattern_list_note[i] = NULL;
    }

    save_animation_states();

    VDP_setHilightShadow(true); // Dim screen
    show_or_hide_interface(false); // Hide interface
    //show_or_hide_enemy_combat_interface(false); // Hide combat interface
    savedStates = hide_interface_sprites(); // Hide interface sprites and save state

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
    show_interface_sprites(savedStates); // Restore HUD sprites visibility
    VDP_setHilightShadow(false); // Relit screen
    restore_animation_states();
    SPR_update();
    VDP_waitVSync();
    player_has_paused = true; // Set pause flag
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
        spr_pattern_list_note[nnote]=SPR_addSpriteSafe(pentsprite, x, 180, TILE_ATTR(PAL2,true,false,false));
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
    // Increment “time since last key”
    ++combatContext.noteTimer;

    // Abort if player started a pattern but then paused too long
    if (combatContext.playerNotes &&
        combatContext.noteTimer > calc_ticks(MAX_PATTERN_WAIT_TIME))
    {
        dprintf(1,"Pattern aborted after %u ticks", combatContext.noteTimer);

        reset_note_queue();                            // hide all 4 icons
        obj_character[active_character].state = STATE_IDLE;
        play_player_pattern_sound(PATTERN_PLAYER_NONE); // play invalid sound

        // Start global lock so the user cannot mash immediately
        combatContext.patternLockTimer = MIN_TIME_BETWEEN_PATTERNS;
        set_idle(); // Reset combat state
    }
}

// Update the face and life counter sprite for the enemies
void update_life_counter(void) {
    u16 life_counter = 0;

    // Check if there's a hurt enemy
    u8 nenemy = ENEMY_NONE;
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.state == STATE_HIT) {
            nenemy = i; // Found an enemy that is hit
            break;
        }
    }
    if (nenemy == ENEMY_NONE) {
        // If no enemy is hit, use the active enemy from combat context
        nenemy = combatContext.activeEnemy;
    }
    
    // If there's no attackers, release the sprites if it exists and return
    if (nenemy == ENEMY_NONE) {
        if (spr_int_life_counter != NULL) {
            SPR_releaseSprite(spr_int_life_counter);
            spr_int_life_counter = NULL;
        }
        return;
    }
    dprintf(3,"Updating life counter for enemy %d", nenemy);

    // Calculate X and Y position for the life counter, just above the enemy
    u16 x = FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.x) + (obj_enemy[nenemy].obj_character.x_size / 2) - 16; // Centered on the enemy. 16 is half the life counter width.
    u16 y = FASTFIX32_TO_INT(obj_enemy[nenemy].obj_character.y) - 8; // 16 pixels above the enemy

    // Get the life counter animation (hitpoints - 1)
    life_counter = obj_enemy[nenemy].hitpoints - 1;
       
    // Load the life counter sprite if it's null
    if (spr_int_life_counter == NULL) {
        spr_int_life_counter  = SPR_addSprite(&int_life_counter_sprite, x, y, TILE_ATTR(PAL2, false, false, false));
    } else {
        SPR_setPosition(spr_int_life_counter, x, y); // Update position if it already exists
    }

    // If enemy is hit, flash the life counter every odd frame
    if (obj_enemy[nenemy].obj_character.state == STATE_HIT) {
        dprintf(2,"Enemy %d hit while checking life counter", nenemy);
        if (frame_counter% 2 == 0) life_counter++; // Flash between life and life-1
        else SPR_setPosition(spr_int_life_counter, x-3, y); // Fix position to avoid flickering
    }

    // Set the life counter sprite's animation based on the life counter (if not already set)
    if (spr_int_life_counter->animInd != life_counter) {
        SPR_setAnim(spr_int_life_counter, life_counter);
    }

    SPR_update(); // Update the sprite engine to reflect changes
}
