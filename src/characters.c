#include <genesis.h>
#include "globals.h"

// Global variable definitions
Entity obj_character[MAX_CHR];
Sprite *spr_chr[MAX_CHR];
Sprite *spr_chr_shadow[MAX_CHR];
u16 active_character;
Entity obj_face[MAX_FACE];
Sprite *spr_face[MAX_FACE];

// Update shadow position for a character
void update_character_shadow(u16 nchar)
{
    s16 base_x;

    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        // Base X depends on character
        switch (nchar)
        {
        case CHR_clio:
            base_x = 4;
            break;
        case CHR_linus:
            base_x = 6;
            break;
        case CHR_xander:
            base_x = 10;
            break;
        default:
            base_x = 6;
            break;
        }

        // Position shadow at the bottom of character's collision box
        s16 shadow_x;
        // Position shadow at the bottom center of character's collision box
        if (obj_character[nchar].flipH) {
            // When facing left, adjust base position to account for flipped sprite
            base_x = obj_character[nchar].x_size - base_x - 24;
        }
        
        shadow_x = obj_character[nchar].x + base_x;  // Center shadow (24/2 = 12)
        s16 shadow_y = obj_character[nchar].y + obj_character[nchar].collision_y_offset - 4;      // Place at bottom (8/2 = 4)
        
        SPR_setPosition(spr_chr_shadow[nchar], shadow_x, shadow_y);
    }
}

// Initialize a character
void init_character(u16 nchar)
{
    u8 npal = PAL1;
    u8 x_size, y_size; // We can get them from the Sprite Definition
    u8 collision_x_offset=4;
    u8 collision_y_offset=63;
    u8 collision_width=36;
    u8 collision_height=2;
    bool drops_shadow=true;
    const SpriteDefinition *nsprite = NULL;

    if (obj_character[nchar].sd == NULL) {
        switch (nchar)
        {
        case CHR_linus:
            nsprite = &linus_sprite;        
            break;
        case CHR_clio:
            nsprite = &clio_sprite;
            break;
        case CHR_xander:
            nsprite = &xander_sprite;
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
        obj_character[nchar] = (Entity) { true, nsprite, 0, 0, x_size, y_size, npal, false, false, ANIM_IDLE, false, collision_x_offset, collision_y_offset, collision_width, collision_height, STATE_IDLE, FALSE, 0, drops_shadow };
    } else {
        nsprite = obj_character[nchar].sd;
        npal = obj_character[nchar].palette;
        obj_character[nchar].active=true;
    }

    spr_chr[nchar] = SPR_addSpriteSafe(nsprite, obj_character[nchar].x, obj_character[nchar].y, 
                                       TILE_ATTR(npal, obj_character[nchar].priority, false, obj_character[nchar].flipH));

    if (spr_chr[nchar] != NULL) {
        SPR_setVisibility(spr_chr[nchar], HIDDEN);
    }

    // Initialize shadow if character drops one
    if (obj_character[nchar].drops_shadow) {
        spr_chr_shadow[nchar] = SPR_addSpriteSafe(&shadow_sprite, 0, 0, TILE_ATTR(PAL1, TRUE, FALSE, FALSE));
        
        if (spr_chr_shadow[nchar] != NULL) {
            SPR_setVisibility(spr_chr_shadow[nchar], HIDDEN);
            SPR_setDepth(spr_chr_shadow[nchar], SPR_MAX_DEPTH); // Shadow always at back
            update_character_shadow(nchar);
        }
    }
}

// Release a character from memory (Just the sprite, keep the Entity)
void release_character(u16 nchar)
{
    obj_character[nchar].active = false;
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
}

// Initialize a face
void init_face(u16 nface)
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
        obj_face[nface] = (Entity) { true, nsprite, 0, 160, 64, 64, npal, false, false, ANIM_IDLE, false, 0, 0, 0, 0, STATE_IDLE, FALSE, 0, false };
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

// Release a face from memory (Just the sprite, keep the Entity)
void release_face(u16 nface)
{
    obj_face[nface].active = false;
    if (spr_face[nface] != NULL)
    {
        SPR_releaseSprite(spr_face[nface]);
        spr_face[nface] = NULL;
    }
}

// Update a character based on every parameter
void update_character(u16 nchar)
{
    SPR_setPosition(spr_chr[nchar],obj_character[nchar].x,obj_character[nchar].y);
    SPR_setPriority(spr_chr[nchar],obj_character[nchar].priority);
    SPR_setVisibility(spr_chr[nchar],obj_character[nchar].visible?VISIBLE:HIDDEN);
    SPR_setHFlip(spr_chr[nchar],obj_character[nchar].flipH);
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
    update_character_shadow(nchar);
}

// Show or hide a character
void show_character(u16 nchar, bool show)
{
    obj_character[nchar].visible=show;
    SPR_setVisibility(spr_chr[nchar],show?VISIBLE:HIDDEN);
    
    // Update shadow visibility if it exists
    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        SPR_setVisibility(spr_chr_shadow[nchar],show?VISIBLE:HIDDEN);
    }
    
    SPR_update();
}

// Change a character's animation
void anim_character(u16 nchar, u8 newanimation)
{
    if (obj_character[nchar].animation!=newanimation) {
        obj_character[nchar].animation=newanimation;
        SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
        SPR_update();
    }
}

// Make a character look to the left (or right)
void look_left(u16 nchar, bool direction_right)
{
    obj_character[nchar].flipH=direction_right;
    SPR_setHFlip (spr_chr[nchar], direction_right);
    SPR_update();
}

// Move a character to a new position
void move_character(u16 nchar, s16 newx, s16 newy)
{
    show_character(nchar, true);
    anim_character(nchar, ANIM_WALK);

    // Look in the appropriate direction
    s16 dx = newx - obj_character[nchar].x;
    if (dx < 0) {
        look_left(nchar, true);
    } else if (dx > 0) {
        look_left(nchar, false);
    }

    move_entity(&obj_character[nchar], spr_chr[nchar], newx, newy);
    anim_character(nchar, ANIM_IDLE);
}

// Move a character to a new position (instantly)
void move_character_instant(u16 nchar,s16 x,s16 y)
{
    y-=obj_character[nchar].y_size; // Now all calculations are relative to the bottom line, not the upper one

    SPR_setPosition(spr_chr[nchar], x, y);
    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
    update_character_shadow(nchar);
    next_frame(false);
}

// Update characters, items and enemies depth
void update_sprites_depth(void)
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
        if (obj_item[i].entity.active==true) {
            if (obj_item[i].is_background) {
                SPR_setDepth(spr_item[i], SPR_MAX_DEPTH); // Background items are always at the back
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

// Follow (or unfollow active character)
void follow_active_character(u16 nchar, bool follow, u8 follow_speed)
{
    obj_character[nchar].follows_character=follow;
    obj_character[nchar].follow_speed=follow_speed;
    obj_character[nchar].state=STATE_FOLLOWING;
    show_character(nchar, true);
}

// Move characters with STATE_FOLLOWING towards the active character
void approach_characters(void)
{
    u16 nchar;
    s16 newx, newy;
    s16 dx, dy;
    bool has_moved;
    u16 distance;

    for (nchar = 0; nchar < MAX_CHR; nchar++) {
        // Skip if this is the active character
        if (nchar == active_character) {
            continue;
        }

        has_moved = false;
        // Check if character is active and in following state
        if (obj_character[nchar].active && obj_character[nchar].follows_character == true) {
            // Delay movement by follow_speed
            if (frame_counter%obj_character[nchar].follow_speed==0) {
                // Calculate direction towards active character
                dx = obj_character[active_character].x - obj_character[nchar].x;
                dy = (obj_character[active_character].y + obj_character[active_character].y_size) - 
                    (obj_character[nchar].y + obj_character[nchar].y_size);

                // Move by 1 pixel in the calculated direction
                newx = obj_character[nchar].x + (dx != 0 ? (dx > 0 ? 1 : -1) : 0);
                newy = obj_character[nchar].y + (dy != 0 ? (dy > 0 ? 1 : -1) : 0);

                // Check distance to active character at new position
                distance = char_distance(nchar, newx, newy, active_character);

                // Start moving when distance >40, keep moving until distance >20
                if ((obj_character[nchar].animation==ANIM_IDLE && distance>40) || (obj_character[nchar].animation==ANIM_WALK && distance>20)) {
                    // Update character position and animation
                    obj_character[nchar].x = newx;
                    obj_character[nchar].y = newy;
                    obj_character[nchar].animation = ANIM_WALK;
                    obj_character[nchar].flipH = (dx < 0);
                    update_character(nchar);
                    has_moved = true;
                }
            }
        }

        // Set to idle if not moved
        if (!has_moved && obj_character[nchar].state == STATE_FOLLOWING) {
            anim_character(nchar, ANIM_IDLE);
        }
    }

    // Update sprite depths after movement
    update_sprites_depth();
}
