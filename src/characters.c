#include <genesis.h>
#include "globals.h"

Entity obj_character[MAX_CHR];                    // Array of character instances with their properties
Sprite *spr_chr[MAX_CHR];                        // Array of character sprites
Sprite *spr_chr_shadow[MAX_CHR];                 // Array of character shadow sprites
Entity obj_face[MAX_FACE];                             // Array of face instances with their properties
Sprite *spr_face[MAX_FACE];                            // Array of face sprites
u16 active_character;                                  // Which character is the active one
bool movement_active;                                  // Can you move ?

void update_character_shadow(u16 nchar)    // Update shadow sprite position based on character position
{
    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        // Position shadow at the bottom of character's collision box
        s16 shadow_x = obj_character[nchar].x;
        s16 shadow_y = obj_character[nchar].y + obj_character[nchar].collision_y_offset - 4;
        
        // Flip shadow if character is looking to the left
        SPR_setHFlip(spr_chr_shadow[nchar], obj_character[nchar].flipH);

        // Set shadow position
        SPR_setPosition(spr_chr_shadow[nchar], shadow_x, shadow_y);
    }
}

void init_character(u16 nchar)    // Create new character instance with sprites and collision
{
    u8 npal = PAL2;
    u8 x_size, y_size;
    u8 collision_x_offset=0,collision_y_offset=0,collision_width=0,collision_height=0;
    bool drops_shadow=true;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_shadow = NULL;

    // Set specific attributes based on character
    switch (nchar)
    {
    case CHR_linus:
        collision_x_offset=4;
        collision_y_offset=95;
        nsprite = &linus_sprite;
        nsprite_shadow = &linus_shadow_sprite;
        break;
    case CHR_clio:
        collision_x_offset=4;
        collision_y_offset=95;
        nsprite = &clio_sprite;
        nsprite_shadow = &clio_shadow_sprite;
        break;
    case CHR_xander:
        collision_x_offset=4;
        collision_y_offset=95;
        nsprite = &xander_sprite;
        nsprite_shadow = &xander_shadow_sprite;
        break;
    case CHR_swan:
        collision_x_offset=4;
        collision_y_offset=95;
        nsprite = &swan_sprite;
        nsprite_shadow = NULL;
        drops_shadow = false;
        break;
    default:
        return;
    }
    
    // Get width and height from the sprite definition
    x_size = nsprite->w; 
    y_size = nsprite->h;
    // Set default collision box if not defined previously
    if (collision_width==0) collision_width=x_size/2; // Half width size
    if (collision_x_offset==0) collision_x_offset=x_size/4; // Centered in X
    if (collision_height==0) collision_height=2; // Two lines height
    if (collision_y_offset==0) collision_y_offset=y_size-1; // At the feet

    // Initialize character with sprite, position, and collision attributes
    obj_character[nchar] = (Entity) { 
        true, nsprite, nsprite_shadow, 0, 0, x_size, y_size, npal, false, false, 
        ANIM_IDLE, false, collision_x_offset, collision_y_offset, 
        collision_width, collision_height, STATE_IDLE, 
        false, 0, drops_shadow
    };

    // Add character sprite if not already present
    if (spr_chr[nchar]==NULL) spr_chr[nchar] = SPR_addSpriteSafe(nsprite, obj_character[nchar].x, obj_character[nchar].y, 
                                       TILE_ATTR(npal, obj_character[nchar].priority, false, obj_character[nchar].flipH));
    
    // Initialize shadow if character drops one
    if (obj_character[nchar].drops_shadow) {
        if (spr_chr_shadow[nchar] == NULL) {
            spr_chr_shadow[nchar] = SPR_addSpriteSafe(nsprite_shadow, 0, 0, TILE_ATTR(npal, TRUE, FALSE, FALSE));
        }
        if (spr_chr_shadow[nchar] != NULL) {
            SPR_setVisibility(spr_chr_shadow[nchar], HIDDEN);
            SPR_setDepth(spr_chr_shadow[nchar], SPR_MAX_DEPTH); // Shadow always at back
            update_character_shadow(nchar);
        }
    }

    // Initially hide character sprites
    SPR_setVisibility(spr_chr[nchar], HIDDEN);
}

void release_character(u16 nchar)    // Free character resources
{
    obj_character[nchar].active = false;
    if (spr_chr[nchar] != NULL) {
        SPR_releaseSprite(spr_chr[nchar]);
        spr_chr[nchar] = NULL;
    }
    if (spr_chr_shadow[nchar] != NULL) {
        SPR_releaseSprite(spr_chr_shadow[nchar]);
        spr_chr_shadow[nchar] = NULL;
    }
}

void init_face(u16 nface)    // Create new face instance with sprite
{
    u8 npal = PAL2;
    u8 x_size, y_size;
    const SpriteDefinition *nsprite = NULL;

    // Set specific attributes based on face
    switch (nface)
    {
    case FACE_linus:
        nsprite = &linus_face_sprite;
        break;
    case FACE_clio:
        nsprite = &clio_face_sprite;
        break;
    case FACE_xander:
        nsprite = &xander_face_sprite;
        break;
    case FACE_swan:
        nsprite = &swan_face_sprite;
        break;
    default:
        return;
    }
    
    // Get width and height from the sprite definition
    x_size = nsprite->w; 
    y_size = nsprite->h;

    // Initialize face with sprite and position
    obj_face[nface] = (Entity) { 
        true, nsprite, NULL, 0, 0, x_size, y_size, npal, false, false, 
        ANIM_IDLE, false, 0, 0, 0, 0, STATE_IDLE, false, 0, false
    };

    // Add face sprite if not already present
    if (spr_face[nface]==NULL) spr_face[nface] = SPR_addSpriteSafe(nsprite, obj_face[nface].x, obj_face[nface].y, 
                                       TILE_ATTR(npal, obj_face[nface].priority, false, obj_face[nface].flipH));
    
    // Initially hide face sprite
    SPR_setVisibility(spr_face[nface], HIDDEN);
}

void release_face(u16 nface)    // Free face resources
{
    obj_face[nface].active = false;
    if (spr_face[nface] != NULL) {
        SPR_releaseSprite(spr_face[nface]);
        spr_face[nface] = NULL;
    }
}

void update_character(u16 nchar)    // Update character sprite properties from current state
{
    SPR_setPosition(spr_chr[nchar], obj_character[nchar].x, obj_character[nchar].y);
    SPR_setPriority(spr_chr[nchar], obj_character[nchar].priority);
    SPR_setVisibility(spr_chr[nchar], obj_character[nchar].visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_chr[nchar], obj_character[nchar].flipH);
    SPR_setAnim(spr_chr[nchar], obj_character[nchar].animation);
    update_character_shadow(nchar);
    SPR_update();
}

void show_character(u16 nchar, bool show)    // Toggle visibility of character and its shadow
{
    obj_character[nchar].visible = show;
    SPR_setVisibility(spr_chr[nchar], show ? VISIBLE : HIDDEN);
    
    // Update shadow visibility if it exists
    if (obj_character[nchar].drops_shadow && spr_chr_shadow[nchar] != NULL) {
        SPR_setVisibility(spr_chr_shadow[nchar], show ? VISIBLE : HIDDEN);
    }
    
    SPR_update();
}

void anim_character(u16 nchar, u8 newanimation)    // Set character animation if different from current
{
    if (obj_character[nchar].animation != newanimation) {
        obj_character[nchar].animation = newanimation;
        SPR_setAnim(spr_chr[nchar], obj_character[nchar].animation);
    }
}

void look_left(u16 nchar, bool left)    // Set character sprite horizontal flip
{
    obj_character[nchar].flipH = left;
    SPR_setHFlip(spr_chr[nchar], left);
    update_character_shadow(nchar);
    SPR_update();
}

void move_character(u16 nchar, s16 newx, s16 newy)    // Move character with walking animation and direction update
{
    show_character(nchar, true);
    anim_character(nchar, ANIM_WALK);

    // Determine direction to face based on movement
    s16 dx = newx - obj_character[nchar].x;
    if (dx < 0) {
        look_left(nchar, true); // Face left
    } else if (dx > 0) {
        look_left(nchar, false); // Face right
    }

    move_entity(&obj_character[nchar], spr_chr[nchar], newx, newy);
    update_character_shadow(nchar);

    anim_character(nchar, ANIM_IDLE);
}

void move_character_instant(u16 nchar, s16 x, s16 y)    // Set character position immediately without animation
{
    y-=obj_character[nchar].y_size; // Adjust y position relative to the bottom line

    SPR_setPosition(spr_chr[nchar], x, y);
    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
    update_character_shadow(nchar);
    next_frame(false);
}

void update_sprites_depth(void)    // Update sprite depth ordering based on Y position
{
    u16 nchar, nenemy;
    u16 nsprite;
    u16 num_sprites;
    u16 sprite_depth[MAX_CHR + MAX_ENEMIES];
    Sprite *sprite_list[MAX_CHR + MAX_ENEMIES];

    // Build list of active sprites and their depths
    num_sprites = 0;
    
    // Add character sprites
    for (nchar = 0; nchar < MAX_CHR; nchar++) {
        if (obj_character[nchar].active && obj_character[nchar].visible) {
            sprite_list[num_sprites] = spr_chr[nchar];
            sprite_depth[num_sprites] = obj_character[nchar].y + obj_character[nchar].y_size;
            num_sprites++;
        }
    }
    
    // Add enemy sprites
    for (nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
        if (obj_enemy[nenemy].obj_character.active && obj_enemy[nenemy].obj_character.visible) {
            sprite_list[num_sprites] = spr_enemy[nenemy];
            sprite_depth[num_sprites] = obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.y_size;
            num_sprites++;
        }
    }

    // Sort sprites by depth (bubble sort)
    for (nsprite = 0; nsprite < num_sprites - 1; nsprite++) {
        for (u16 j = 0; j < num_sprites - nsprite - 1; j++) {
            if (sprite_depth[j] > sprite_depth[j + 1]) {
                // Swap depths
                u16 temp_depth = sprite_depth[j];
                sprite_depth[j] = sprite_depth[j + 1];
                sprite_depth[j + 1] = temp_depth;
                
                // Swap sprites
                Sprite *temp_sprite = sprite_list[j];
                sprite_list[j] = sprite_list[j + 1];
                sprite_list[j + 1] = temp_sprite;
            }
        }
    }

    // Update sprite priorities based on sorted order
    for (nsprite = 0; nsprite < num_sprites; nsprite++) {
        SPR_setPriority(sprite_list[nsprite], nsprite);
    }
}

void follow_active_character(u16 nchar, bool follow, u8 follow_speed)    // Set character to follow active character
{
    obj_character[nchar].follows_character = follow;
    obj_character[nchar].follow_speed = follow_speed;
    obj_character[nchar].state = STATE_FOLLOWING;
}

void approach_characters(void)    // Update following characters' positions
{
    u16 nchar;
    s16 newx, newy;
    s16 dx, dy;
    u16 collision_result;
    bool has_moved;

    for (nchar = 0; nchar < MAX_CHR; nchar++) {
        has_moved=false;
        if (obj_character[nchar].follows_character == true) { // Check if this character follows others
            if (frame_counter%obj_character[nchar].follow_speed==0) { // Move at character's specific speed
                // Calculate direction towards active character
                dx = obj_character[active_character].x - obj_character[nchar].x;
                dy = (obj_character[active_character].y + obj_character[active_character].y_size) - 
                    (obj_character[nchar].y + obj_character[nchar].y_size);

                // Move by 1 pixel in the calculated direction
                newx = obj_character[nchar].x + (dx != 0 ? (dx > 0 ? 1 : -1) : 0);
                newy = obj_character[nchar].y + (dy != 0 ? (dy > 0 ? 1 : -1) : 0);

                // Check for collision at new position
                collision_result = detect_char_collision(nchar, newx, newy);

                // Move the character if there's no collision
                if (collision_result == CHR_NONE) {
                    obj_character[nchar].x = newx;
                    obj_character[nchar].y = newy;
                    obj_character[nchar].animation=ANIM_WALK;
                    obj_character[nchar].flipH=(dx<0);
                    update_character(nchar);
                    has_moved=true;
                }
            }
        }
        if (!has_moved && obj_character[nchar].state == STATE_FOLLOWING) {
            anim_character(nchar,ANIM_IDLE); // Set to idle if not moved
        }
    }
}
