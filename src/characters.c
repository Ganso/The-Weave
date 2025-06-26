#include "globals.h"

Entity obj_character[MAX_CHR];         // Array of game characters with their properties
Sprite *spr_chr[MAX_CHR];             // Array of character sprites
Sprite *spr_chr_shadow[MAX_CHR];      // Array of character shadow sprites
u16 active_character;                 // Currently controlled character ID
Entity obj_face[MAX_FACE];            // Array of character face entities for dialogs
Sprite *spr_face[MAX_FACE];           // Array of character face sprites

void update_character_shadow(u16 nchar)    // Update shadow sprite position based on character position
{
    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        // Position shadow at the bottom of character's collision box
        s16 shadow_x = obj_character[nchar].x;  // Center shadow (24/2 = 12)
        s16 shadow_y = obj_character[nchar].y + obj_character[nchar].collision_y_offset - 4;      // Place at bottom (8/2 = 4)
        
        // Flip shadow if character is looking to the left
        SPR_setHFlip(spr_chr_shadow[nchar], obj_character[nchar].flipH);

        // Set shadow position
        SPR_setPosition(spr_chr_shadow[nchar], shadow_x, shadow_y);
    }
}

void init_character(u16 nchar)    // Create new character instance with sprites and collision
{
    u8 npal = PAL1;
    u8 x_size, y_size; // We can get them from the Sprite Definition
    u8 collision_x_offset=0;
    u8 collision_y_offset=0;
    u8 collision_width=0;
    u8 collision_height=0;
    f16 velocity = FIX16_ONE; // Default velocity for characters
    bool drops_shadow=true;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_shadow = NULL;

    dprintf(2,"Initializing character %d\n", nchar);

    if (obj_character[nchar].sd == NULL) {
        switch (nchar)
        {
        case CHR_linus:
            if (player_has_rod) nsprite = &linus_sprite;
            else nsprite = &linus_norod_sprite;
            nsprite_shadow = &linus_shadow_sprite;
            velocity = FIX16_ONE * 1.5 ; // Linus is faster
            break;
        case CHR_clio:
            nsprite = &clio_sprite;
            nsprite_shadow = &clio_shadow_sprite;
            velocity = FIX16_ONE * 3 / 4; // Clio is slower
            break;
        case CHR_xander:
            nsprite = &xander_sprite;
            nsprite_shadow = &xander_shadow_sprite;
            break;
        case CHR_swan:
            nsprite = &swan_sprite;
            drops_shadow = false;
            break;
        default:
            return; 
        }
        
        x_size=nsprite->w; // Get width and height from the Sprite Definition
        y_size=nsprite->h;

        // Set default collision box if not defined
        if (collision_width==0) collision_width=x_size/2; // Half width size
        if (collision_x_offset==0) collision_x_offset=x_size/4; // Centered in X
        if (collision_height==0) collision_height=2; // Two lines height
        if (collision_y_offset==0) collision_y_offset=y_size-1; // At the feet

        obj_character[nchar] = (Entity) { true, nsprite, nsprite_shadow,
            0, 0, INT_TO_FIX16(0), INT_TO_FIX16(0),
            x_size, y_size, npal, false, false, ANIM_IDLE, false,
            collision_x_offset, collision_y_offset, collision_width, collision_height,
            STATE_IDLE, FALSE, velocity, drops_shadow, 0 };
    } else {
        nsprite = obj_character[nchar].sd;
        nsprite_shadow = obj_character[nchar].sd_shadow;
        npal = obj_character[nchar].palette;
        obj_character[nchar].active=true;
    }

    dprintf(2,"Adding sprite for character %d at (%d, %d)\n", nchar, obj_character[nchar].x, obj_character[nchar].y);
    spr_chr[nchar] = SPR_addSpriteSafe(nsprite, obj_character[nchar].x, obj_character[nchar].y, 
                                       TILE_ATTR(npal, obj_character[nchar].priority, false, obj_character[nchar].flipH));

    if (spr_chr[nchar] != NULL) {
        SPR_setVisibility(spr_chr[nchar], HIDDEN);
    }

    // Initialize shadow if character drops one
    if (obj_character[nchar].drops_shadow) {
        dprintf(2,"Adding shadow sprite for character %d\n", nchar);
        spr_chr_shadow[nchar] = SPR_addSpriteSafe(nsprite_shadow, 0, 0, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
        
        if (spr_chr_shadow[nchar] != NULL) {
            SPR_setVisibility(spr_chr_shadow[nchar], HIDDEN);
            SPR_setDepth(spr_chr_shadow[nchar], SPR_MAX_DEPTH); // Shadow always at back
            update_character_shadow(nchar);
        }
    }
}

void release_character(u16 nchar)    // Free character sprite resources but keep entity data
{
    obj_character[nchar].active = false;
    obj_character[nchar].sd = NULL;
    obj_character[nchar].sd_shadow = NULL;
    if (spr_chr[nchar] != NULL)
    {
        SPR_releaseSprite(spr_chr[nchar]);
        spr_chr[nchar] = NULL;
    }
    
    // Release shadow if it exists
    if (spr_chr_shadow[nchar] != NULL)
    {
        SPR_releaseSprite(spr_chr_shadow[nchar]);
        spr_chr_shadow[nchar] = NULL;
    }
    
    // Update sprite engine after releasing sprites
    SPR_update();
}

void init_face(u16 nface)    // Create new character face sprite for dialogs
{
    u8 npal = PAL1;
    const SpriteDefinition *nsprite = NULL;

    if (obj_face[nface].sd == NULL) { // Object never initialized
        switch (nface)
        {
        case CHR_linus:
            nsprite = &linus_face_sprite;        
            break;
        case CHR_clio:
            nsprite = &clio_face_sprite;
            break;
        case CHR_xander:
            nsprite = &xander_face_sprite;
            break;
        case CHR_swan:
            nsprite = &swan_face_sprite;
            break;
        default:
            return;
        }
        obj_face[nface] = (Entity) { true, nsprite, NULL,
            0, 160, INT_TO_FIX16(0), INT_TO_FIX16(160),
            64, 64, npal, false, false, ANIM_IDLE, false,
            0, 0, 0, 0, STATE_IDLE, FALSE,
            INT_TO_FIX16(1), false, 0 };
    } else {
        nsprite = obj_face[nface].sd;
        obj_face[nface].active=true;
    }

    spr_face[nface] = SPR_addSpriteSafe(nsprite, obj_face[nface].x, obj_face[nface].y, 
                                        TILE_ATTR(obj_face[nface].palette, obj_face[nface].priority, false, obj_face[nface].flipH));
    
    if (spr_face[nface] != NULL) {
        SPR_setVisibility(spr_face[nface], HIDDEN);
        SPR_setDepth(spr_face[nface], SPR_MIN_DEPTH); // Faces are above any other sprite
    }
}

void release_face(u16 nface)    // Free face sprite resources but keep entity data
{
    obj_face[nface].active = false;
    if (spr_face[nface] != NULL)
    {
        SPR_releaseSprite(spr_face[nface]);
        spr_face[nface] = NULL;
    }
}

void update_character(u16 nchar)    // Update character sprite properties from current state
{
    SPR_setPosition(spr_chr[nchar],obj_character[nchar].x,obj_character[nchar].y);
    SPR_setPriority(spr_chr[nchar],obj_character[nchar].priority);
    SPR_setVisibility(spr_chr[nchar],obj_character[nchar].visible?VISIBLE:HIDDEN);
    SPR_setHFlip(spr_chr[nchar],obj_character[nchar].flipH);
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
    update_character_shadow(nchar);
}

void show_character(u16 nchar, bool show)    // Toggle visibility of character and its shadow
{
    obj_character[nchar].visible=show;
    SPR_setVisibility(spr_chr[nchar],show?VISIBLE:HIDDEN);
    
    // Update shadow visibility if it exists
    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        SPR_setVisibility(spr_chr_shadow[nchar],show?VISIBLE:HIDDEN);
    }
    
    SPR_update();
}

void anim_character(u16 nchar, u8 newanimation)    // Set character animation if different from current
{
    if (obj_character[nchar].animation!=newanimation) {
        obj_character[nchar].animation=newanimation;
        SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
        SPR_update();
    }
}

void look_left(u16 nchar, bool direction_right)    // Set character sprite horizontal flip
{
    obj_character[nchar].flipH=direction_right;
    SPR_setHFlip (spr_chr[nchar], direction_right);
    SPR_update();
}

void move_character(u16 nchar, s16 newx, s16 newy)    // Move character with walking animation and direction update
{
    show_character(nchar, true);
    obj_character[nchar].state=STATE_WALKING;

    // Look in the appropriate direction
    s16 dx = newx - obj_character[nchar].x;
    if (dx < 0) {
        look_left(nchar, true);
    } else if (dx > 0) {
        look_left(nchar, false);
    }

    move_entity(&obj_character[nchar], spr_chr[nchar], newx, newy);
    obj_character[nchar].state=STATE_IDLE; // Set state to idle after moving
}

void move_character_instant(u16 nchar,s16 x,s16 y)    // Set character position immediately without animation
{
    y-=obj_character[nchar].y_size; // Now all calculations are relative to the bottom line, not the upper one

    SPR_setPosition(spr_chr[nchar], x, y);
    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
    obj_character[nchar].fx = INT_TO_FIX16(x);
    obj_character[nchar].fy = INT_TO_FIX16(y);
    update_character_shadow(nchar);
    next_frame(false);
}

void update_sprites_depth(void)    // Sort sprite layers based on Y position for proper overlap
{
    u16 i;

    // Update character depth
    for (i = 0; i < MAX_CHR; i++) {
        if (obj_character[i].active==true) {
            SPR_setDepth(spr_chr[i], -obj_character[i].y-obj_character[i].y_size); // Negative of the bottom line of the sprite
        }
    }

    // Update items depth
    for (i = 0; i < MAX_ITEMS; i++) {
        if (obj_item[i].entity.active==true && spr_item[i]!=NULL) {
            if (obj_item[i].check_depth==FORCE_BACKGROUND) {
                SPR_setDepth(spr_item[i], SPR_MAX_DEPTH); // Background items are always at the back
            } else if (obj_item[i].check_depth==FORCE_FOREGROUND) {
                SPR_setDepth(spr_item[i], SPR_MIN_DEPTH+100); // Foreground items are always at the front (add 100 so it doesn't interfere with frontend interface items)
            } else {
                SPR_setDepth(spr_item[i], -obj_item[i].entity.y-obj_item[i].entity.y_size); // Negative of the bottom line of the sprite
            }
        }
    }

    // Update enemies depth
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active==true) {
            SPR_setDepth(spr_enemy[i], -obj_enemy[i].obj_character.y-obj_enemy[i].obj_character.y_size); // Negative of the bottom line of the sprite
        }
    }
}

void follow_active_character(u16 nchar, bool follow)    // Set character to follow active character
{
    obj_character[nchar].follows_character=follow;
    obj_character[nchar].state=STATE_IDLE;
    show_character(nchar, true);
}

void approach_characters(void)    // Move NPCs that follow the hero
{
    if (followers_moved_by_scroll)
    {
        followers_moved_by_scroll = FALSE;
        return;
    }
    u16 nchar;
    s16 newx, newy;
    s16 dx,  dy;
    bool has_moved;
    u16 distance;

    dprintf(3,"Approaching characters\n");

    for (nchar = 0; nchar < MAX_CHR; nchar++)
    {
        dprintf(3,"Approaching character %d\n", nchar);

        // Skip the active character
        if (nchar == active_character) continue;

        // Only NPCs flagged as followers
        if (!obj_character[nchar].active ||
            !obj_character[nchar].follows_character)               continue;


        dprintf(3,"Character %d is following\n", nchar);

        has_moved=false;

        // Calculate new position towards the active character (1 px step)
        dx = obj_character[active_character].x -
             obj_character[nchar].x;
        dy = (obj_character[active_character].y +
              obj_character[active_character].y_size) -
             (obj_character[nchar].y +
              obj_character[nchar].y_size);

        if (dx)
            obj_character[nchar].fx += (dx > 0 ? obj_character[nchar].velocity : -obj_character[nchar].velocity);
        if (dy)
            obj_character[nchar].fy += (dy > 0 ? obj_character[nchar].velocity : -obj_character[nchar].velocity);

        newx = FIX16_TO_INT(obj_character[nchar].fx);
        newy = FIX16_TO_INT(obj_character[nchar].fy);

        // Distance to the active character if we accept the new position
        distance = char_distance(nchar, newx, newy, active_character);

        dprintf(3,"Character %d distance to active character: %d\n", nchar, distance);

        // Should we move?
        // If idle and too far, start walking. If walking, continue until close enough.
        if (  (obj_character[nchar].state == STATE_IDLE   &&
               distance > MAX_FOLLOW_DISTANCE)            ||
              (obj_character[nchar].state == STATE_WALKING &&
               distance > MIN_FOLLOW_DISTANCE) )
        {
            dprintf(3,"Character %d moving to (%d, %d)\n", nchar, newx, newy);

            // Update entity position and keep fixed coordinates in sync
            obj_character[nchar].x     = newx;
            obj_character[nchar].y     = newy;
            obj_character[nchar].fx    = INT_TO_FIX16(newx);
            obj_character[nchar].fy    = INT_TO_FIX16(newy);
            obj_character[nchar].flipH = (dx < 0);

            // Update sprite position and properties
            update_character(nchar);

            // If we are not already walking, set state to WALKING
            if (obj_character[nchar].state == STATE_IDLE)
                obj_character[nchar].state = STATE_WALKING;

            has_moved = true;
        }

        // If we are close enough or didn't move, set state to IDLE
        if (distance <= MIN_FOLLOW_DISTANCE || !has_moved)
        {
            obj_character[nchar].state = STATE_IDLE;
        }
    }

    // Update all sprites depth after moving characters, so they are drawn in the correct order
    update_sprites_depth();
}


void reset_character_animations()
{
    for (u16 i = 0; i < MAX_CHR; i++)
    {
        if (obj_character[i].active && i != active_character)
        {
            obj_character[i].state = STATE_IDLE;
        }
    }

    for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++)
    {
        if (obj_enemy[nenemy].obj_character.active)
        {
            // anim_enemy(nenemy, ANIM_IDLE);
            // obj_enemy[nenemy].obj_character.state = STATE_IDLE;
        }
    }
}

// Update the character's animation based on its current state
void update_character_animations(void) {

    for (u16 chr = 0; chr < MAX_CHR; chr++) {
        if (!obj_character[chr].active) continue;

        // If there's an active pattern in combat state, play magic animation
        // even if the character is not in PATTERN_EFFECT state (solo protagonista)
        if (chr == active_character &&
            combat_state == COMBAT_STATE_PLAYER_EFFECT &&
            combatContext.activePattern != PATTERN_HIDE)
        {
            if (obj_character[chr].animation != ANIM_MAGIC) {
                anim_character(chr, ANIM_MAGIC);
            }
            continue;   // Skip further checks for this character
        }

        switch (obj_character[chr].state) {
            case STATE_WALKING:
                if (obj_character[chr].animation != ANIM_WALK) {
                    obj_character[chr].animation = ANIM_WALK;
                    update_character(chr);
                }
                break;
            case STATE_IDLE:
                if (obj_character[chr].animation != ANIM_IDLE &&
                    (
                        SPR_isAnimationDone(spr_chr[chr]) ||
                        obj_character[chr].animation == ANIM_WALK
                    )) {
                    anim_character(chr, ANIM_IDLE); // Let any animation finish before setting to idle (except WALKING)
                }
                break;
            case STATE_PLAYING_NOTE:
                if (obj_character[chr].animation != ANIM_ACTION) {
                    anim_character(chr, ANIM_ACTION);
                }
                break;
            case STATE_PATTERN_EFFECT:
                if (chr == active_character && combatContext.activePattern == PATTERN_HIDE) {
                    // Keep current animation (idle or walking) while hidden
                } else if (obj_character[chr].animation != ANIM_MAGIC) {
                    anim_character(chr, ANIM_MAGIC);
                }
                break;
            case STATE_PATTERN_EFFECT_FINISH:
                if (obj_character[chr].animation != ANIM_IDLE) {
                    anim_character(chr, ANIM_IDLE);
                }
                obj_character[chr].state = STATE_IDLE;
                break;
        case STATE_HIT:
            if (obj_character[chr].animation != ANIM_HURT)
                anim_character(chr, ANIM_HURT);

            // Count-down stun
            if (obj_character[chr].modeTimer)
                --obj_character[chr].modeTimer;
            else {
                obj_character[chr].state = STATE_IDLE;
                anim_character(chr, ANIM_IDLE);
                show_or_hide_interface(false);
                talk_dialog(&dialogs[ACT1_DIALOG3][A1D3_THAT_HURTS]); // (ES) "Eso ha dolido" - (EN) "That hurts"
                talk_dialog(&dialogs[ACT1_DIALOG3][A1D3_TRY_HIDE_OR_THUNDER]); // (ES) "Puedo probar a esconderme|o tratar de invocar|al trueno" - (EN) "I could try to hide|or attempt to summon|the thunder"
                show_or_hide_interface(true);       
                }
            break;
            default:
                break;
        }
    }
}



