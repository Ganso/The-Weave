#include <genesis.h>
#include "globals.h"

// Global variable definitions
Enemy obj_enemy[MAX_ENEMIES];
Sprite *spr_enemy[MAX_ENEMIES];
Sprite *spr_enemy_face[MAX_ENEMIES];
Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES];


// Initialize enemy classes with their specific attributes
void init_enemy_classes(void)
{
    obj_enemy_class[ENEMY_CLS_BADBOBBIN]=(Enemy_Class) {2, {true, false}, false, 0}; // 2 HP, can use electric pattern, don't follow
    obj_enemy_class[ENEMY_CLS_3HEADMONKEY]=(Enemy_Class) {3, {false, true}, true, 3}; // 3 HP, can use bite pattern, follows at speed 3
}

// Initialize an enemy with specific attributes based on its class
void init_enemy(u16 numenemy, u16 class)
{
    u16 i;
    u8 npal = PAL3;
    u8 x_size, y_size;
    u8 collision_x_offset=0,collision_y_offset=0,collision_width=0,collision_height=0;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_face = NULL;
 
    obj_enemy[numenemy].class_id=class;
    obj_enemy[numenemy].class=obj_enemy_class[class];
    obj_enemy[numenemy].hitpoints=obj_enemy_class[class].max_hitpoints;

    // Set specific attributes based on enemy class
    switch (class)
    {
    case ENEMY_CLS_BADBOBBIN:
        collision_x_offset=4;
        collision_y_offset=95;
        nsprite = &badbobbin_sprite;
        nsprite_face = &badbobbin_sprite_face;
        break;
    case ENEMY_CLS_3HEADMONKEY:
        collision_x_offset = 20;
        collision_y_offset = 55;
        collision_width = 20;
        collision_height = 2;
        nsprite = &three_head_monkey_sprite;
        nsprite_face = &three_head_monkey_sprite_face;
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

    // Initialize enemy character with sprite, position, and collision attributes
    obj_enemy[numenemy].obj_character = (Entity) { 
        true, nsprite, 0, 0, x_size, y_size, npal, false, false, 
        ANIM_IDLE, false, collision_x_offset, collision_y_offset, 
        collision_width, collision_height, STATE_IDLE, 
        obj_enemy_class[class].follows_character, obj_enemy_class[class].follow_speed
    };

    // Add enemy sprite if not already present
    if (spr_enemy[numenemy]==NULL) spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, obj_enemy[numenemy].obj_character.x, obj_enemy[numenemy].obj_character.y, 
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    // Add enemy face sprite if not already present
    if (spr_enemy_face[numenemy]==NULL) spr_enemy_face[numenemy] = SPR_addSpriteSafe(nsprite_face, 198, 178, TILE_ATTR(npal, false, false, false));

    // Initially hide enemy sprites
    SPR_setVisibility(spr_enemy[numenemy], HIDDEN);
    SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
    
    // Reset last pattern times
    for (i=0;i<MAX_PATTERN_ENEMY;i++) obj_enemy[numenemy].last_pattern_time[i]=0;
}

// Release an enemy's sprites and clean up combat state
void release_enemy(u16 nenemy)
{
    // Clean up combat state if this enemy was attacking
    if (enemy_attacking == nenemy) {
        enemy_attacking = ENEMY_NONE;
        enemy_attack_pattern_notes = 0;
        enemy_attack_time = 0;
        enemy_attack_effect_in_progress = false;
        
        // Clean up note indicators
        for (u8 note = 0; note < 6; note++) {
            if (enemy_note_active[note]) {
                show_enemy_note(note + 1, false, false);
                enemy_note_active[note] = false;
            }
        }
    }

    // Release enemy sprites
    obj_enemy[nenemy].obj_character.active = false;
    if (spr_enemy[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy[nenemy]);
        spr_enemy[nenemy] = NULL;
    }
    if (spr_enemy_face[nenemy] != NULL) {
        SPR_releaseSprite(spr_enemy_face[nenemy]);
        spr_enemy_face[nenemy] = NULL;
    }
}

// Update an enemy's sprite based on its current attributes
void update_enemy(u16 nenemy)
{
    SPR_setPosition(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.x, obj_enemy[nenemy].obj_character.y);
    SPR_setPriority(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.priority);
    SPR_setVisibility(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.flipH);
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    SPR_update();
}

// Show or hide an enemy's sprite
void show_enemy(u16 nenemy, bool show)
{
    obj_enemy[nenemy].obj_character.visible = show;
    SPR_setVisibility(spr_enemy[nenemy], show ? VISIBLE : HIDDEN);
    SPR_update();
}

// Change an enemy's animation if it's different from the current one
void anim_enemy(u16 nenemy, u8 newanimation)
{
    if (obj_enemy[nenemy].obj_character.animation != newanimation) {
        obj_enemy[nenemy].obj_character.animation = newanimation;
        SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    }
}

// Make an enemy face left or right
void look_enemy_left(u16 nenemy, bool direction_right)
{
    obj_enemy[nenemy].obj_character.flipH = direction_right;
    SPR_setHFlip(spr_enemy[nenemy], direction_right);
    SPR_update();
}

// Move an enemy to a new position with animation
void move_enemy(u16 nenemy, s16 newx, s16 newy)
{
    show_enemy(nenemy, true);
    anim_enemy(nenemy, ANIM_WALK);

    // Determine direction to face based on movement
    s16 dx = newx - obj_enemy[nenemy].obj_character.x;
    if (dx < 0) {
        look_enemy_left(nenemy, true); // Face left
    } else if (dx > 0) {
        look_enemy_left(nenemy, false); // Face right
    }

    move_entity(&obj_enemy[nenemy].obj_character, spr_enemy[nenemy], newx, newy);

    anim_enemy(nenemy, ANIM_IDLE);
}

// Move an enemy to a new position instantly without animation
void move_enemy_instant(u16 nenemy, s16 x, s16 y)
{
    y-=obj_enemy[nenemy].obj_character.y_size; // Adjust y position relative to the bottom line

    SPR_setPosition(spr_enemy[nenemy], x, y);
    obj_enemy[nenemy].obj_character.x = x;
    obj_enemy[nenemy].obj_character.y = y;
    next_frame(false);
}

// Move enemies towards the active character during combat
void approach_enemies(void)
{
    u16 nenemy;
    s16 newx, newy;
    s16 dx, dy;
    u16 collision_result;
    bool has_moved;

    if (is_combat_active == true && player_pattern_effect_in_progress != PTRN_HIDE) { // Only move enemies during combat when the player is not hidden
        for (nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            has_moved=false;
            if (obj_enemy[nenemy].obj_character.follows_character == true) { // Check if this enemy type follows characters
                if (frame_counter%obj_enemy[nenemy].obj_character.follow_speed==0) { // Move at enemy's specific speed
                    // Calculate direction towards active character
                    dx = obj_character[active_character].x - obj_enemy[nenemy].obj_character.x;
                    dy = (obj_character[active_character].y + obj_character[active_character].y_size) - 
                        (obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.y_size);

                    // Move by 1 pixel in the calculated direction
                    newx = obj_enemy[nenemy].obj_character.x + (dx != 0 ? (dx > 0 ? 1 : -1) : 0);
                    newy = obj_enemy[nenemy].obj_character.y + (dy != 0 ? (dy > 0 ? 1 : -1) : 0);

                    // Check for collision at new position
                    collision_result = detect_enemy_char_collision(nenemy, newx, newy);

                    // Move the enemy if there's no collision and it's not currently attacking
                    if (collision_result == CHR_NONE && enemy_attacking==ENEMY_NONE) {
                        obj_enemy[nenemy].obj_character.x = newx;
                        obj_enemy[nenemy].obj_character.y = newy;
                        obj_enemy[nenemy].obj_character.animation=ANIM_WALK;
                        obj_enemy[nenemy].obj_character.flipH=(dx<0);
                        update_enemy(nenemy);
                        has_moved=true;
                    }
                if (has_moved==false && enemy_attacking!=nenemy) anim_enemy(nenemy,ANIM_IDLE); // Set to idle if not moved and not attacking
                }
            }
        }
    }
}
