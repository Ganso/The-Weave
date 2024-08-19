#include <genesis.h>
#include "globals.h"

// Initialize enemy pattern
void init_enemy_patterns(void)
{
    obj_Pattern_Enemy[PTRN_EN_ELECTIC]=(Pattern_Enemy) {4, {1,2,3,4}, 200};
    obj_Pattern_Enemy[PTRN_EN_BITE]=(Pattern_Enemy) {3, {2,3,2,NULL}, 200};
}

// initialize enemy classes
void init_enemy_classes(void)
{
    obj_enemy_class[ENEMY_CLS_BADBOBBIN]=(Enemy_Class) {2, {true, false}, false, NULL};
    obj_enemy_class[ENEMY_CLS_3HEADMONKEY]=(Enemy_Class) {3, {false, true}, true, 2};
}

// Initialize an enemy
void init_enemy(u16 numenemy, u16 class)
{
    u16 i;
    u8 npal = PAL3;
    u8 x_size, y_size;
    u8 collision_x_offset,collision_y_offset,collision_width,collision_height;
    const SpriteDefinition *nsprite = NULL;
    const SpriteDefinition *nsprite_face = NULL;
 
    obj_enemy[numenemy].class=obj_enemy_class[class];
    obj_enemy[numenemy].hitpoints=obj_enemy_class[class].max_hitpoints;

    switch (class)
    {
    case ENEMY_CLS_BADBOBBIN:
        x_size = 48;
        y_size = 96;
        collision_x_offset=4;
        collision_y_offset=95;
        collision_width=32;
        collision_height=2;
        nsprite = &badbobbin_sprite;
        nsprite_face = &badbobbin_sprite_face;
        break;
    case ENEMY_CLS_3HEADMONKEY:
        x_size = 64;
        y_size = 56;
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
    // * Sprite definition, x, y, palette, priority, flipH, animation, visible
    obj_enemy[numenemy].obj_character = (Entity) { true, nsprite, 0, 0, x_size, y_size, npal, false, false, ANIM_IDLE, false, collision_x_offset, collision_y_offset, collision_width, collision_height };

    spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, obj_enemy[numenemy].obj_character.x, obj_enemy[numenemy].obj_character.y, 
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    spr_enemy_face[numenemy] = SPR_addSpriteSafe(nsprite_face, 198, 178, TILE_ATTR(npal, false, false, false));

    SPR_setVisibility(spr_enemy[numenemy], HIDDEN);
    SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
    
    for (i=0;i<MAX_PATTERN_ENEMY;i++) obj_enemy[numenemy].last_pattern_time[i]=0;
}

// Release an enemy from memory (Just the sprite, keep the Enemy struct)
void release_enemy(u16 nenemy)
{
    obj_enemy[nenemy].obj_character.active=false;
    if (spr_enemy[nenemy] != NULL)
    {
        SPR_releaseSprite(spr_enemy[nenemy]);
        spr_enemy[nenemy] = NULL;
    }
    if (spr_enemy_face[nenemy] != NULL)
    {
        SPR_releaseSprite(spr_enemy_face[nenemy]);
        spr_enemy_face[nenemy] = NULL;
    }
}

// Update an enemy based on every parameter
void update_enemy(u16 nenemy)
{
    SPR_setPosition(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.x, obj_enemy[nenemy].obj_character.y);
    SPR_setPriority(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.priority);
    SPR_setVisibility(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.flipH);
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    SPR_update();
}

// Show or hide an enemy
void show_enemy(u16 nenemy, bool show)
{
    obj_enemy[nenemy].obj_character.visible = show;
    SPR_setVisibility(spr_enemy[nenemy], show ? VISIBLE : HIDDEN);
    SPR_update();
}

// Change an enemy's animation
void anim_enemy(u16 nenemy, u8 newanimation)
{
    if (obj_enemy[nenemy].obj_character.animation != newanimation) {
        obj_enemy[nenemy].obj_character.animation = newanimation;
        SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    }
}

// Make an enemy look to the left (or right)
void look_enemy_left(u16 nenemy, bool direction_right)
{
    obj_enemy[nenemy].obj_character.flipH = direction_right;
    SPR_setHFlip(spr_enemy[nenemy], direction_right);
    SPR_update();
}

// Move an enemy to a new position
void move_enemy(u16 nenemy, s16 newx, s16 newy)
{
    show_enemy(nenemy, true);
    anim_enemy(nenemy, ANIM_WALK);

    // Look in the appropriate direction
    s16 dx = newx - obj_enemy[nenemy].obj_character.x;
    if (dx < 0) {
        look_enemy_left(nenemy, true);
    } else if (dx > 0) {
        look_enemy_left(nenemy, false);
    }

    move_entity(&obj_enemy[nenemy].obj_character, spr_enemy[nenemy], newx, newy);

    anim_enemy(nenemy, ANIM_IDLE);
}

// Move an enemy to a new position (instantly)
void move_enemy_instant(u16 nenemy, s16 x, s16 y)
{
    y-=obj_enemy[nenemy].obj_character.y_size; // Now all calculations are relative to the bottom line, not the upper one

    SPR_setPosition(spr_enemy[nenemy], x, y);
    obj_enemy[nenemy].obj_character.x = x;
    obj_enemy[nenemy].obj_character.y = y;
    next_frame();
}

// Detect collisions between an enemy and every character, given some new coordinates
u16 detect_enemy_collision(u16 nenemy, u16 x, u8 y)
{
    u16 nchar;

    if (obj_enemy[nenemy].obj_character.active == true) {
        // Compute enemy collision box
        u16 enemy_col_x1, enemy_col_x2;
        if (obj_enemy[nenemy].obj_character.flipH) {
            enemy_col_x1 = x + obj_enemy[nenemy].obj_character.x_size - obj_enemy[nenemy].obj_character.collision_x_offset - obj_enemy[nenemy].obj_character.collision_width;
            enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
        } else {
            enemy_col_x1 = x + obj_enemy[nenemy].obj_character.collision_x_offset;
            enemy_col_x2 = enemy_col_x1 + obj_enemy[nenemy].obj_character.collision_width;
        }
        u8 enemy_col_y1 = y + obj_enemy[nenemy].obj_character.collision_y_offset;
        u8 enemy_col_y2 = enemy_col_y1 + obj_enemy[nenemy].obj_character.collision_height;

        // Check collision with characters
        for (nchar = 0; nchar < MAX_CHR; nchar++) {
            if (obj_character[nchar].active == true) {
                // Compute character collision box
                u16 char_col_x1, char_col_x2;
                if (obj_character[nchar].flipH) {
                    char_col_x1 = obj_character[nchar].x + obj_character[nchar].x_size - obj_character[nchar].collision_x_offset - obj_character[nchar].collision_width;
                    char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
                } else {
                    char_col_x1 = obj_character[nchar].x + obj_character[nchar].collision_x_offset;
                    char_col_x2 = char_col_x1 + obj_character[nchar].collision_width;
                }
                u8 char_col_y1 = obj_character[nchar].y + obj_character[nchar].collision_y_offset;
                u8 char_col_y2 = char_col_y1 + obj_character[nchar].collision_height;

                // Check collision
                if (enemy_col_x1 < char_col_x2 &&
                    enemy_col_x2 > char_col_x1 &&
                    enemy_col_y1 < char_col_y2 &&
                    enemy_col_y2 > char_col_y1) {
                    return nchar;
                }
            }
        }
    }
    
    // No collision detected
    return CHR_NONE;
}

// Approach enemies to active character
void approach_enemies(void)
{
    u16 nenemy;
    s16 newx, newy;
    s16 dx, dy;
    u16 collision_result;
    bool has_moved;

    if (is_combat_active == true && pattern_effect_in_progress != PTRN_HIDE) { // We are in combat. Player is not hidden
        for (nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            has_moved=false;
            if (obj_enemy[nenemy].class.follows_character == true) { // Enemy can follow characters
                if (random_seed%obj_enemy[nenemy].class.follow_speed==0) {
                    // Calculate direction to move towards active character
                    dx = obj_character[active_character].x - obj_enemy[nenemy].obj_character.x;
                    dy = (obj_character[active_character].y + obj_character[active_character].y_size) - 
                        (obj_enemy[nenemy].obj_character.y + obj_enemy[nenemy].obj_character.y_size);

                    // Determine new position (approach by 1 pixel)
                    newx = obj_enemy[nenemy].obj_character.x + (dx != 0 ? (dx > 0 ? 1 : -1) : 0);
                    newy = obj_enemy[nenemy].obj_character.y + (dy != 0 ? (dy > 0 ? 1 : -1) : 0);

                    // Check for collision at new position
                    collision_result = detect_enemy_collision(nenemy, newx, newy);

                    // If there's no collision, move the enemy
                    if (collision_result == CHR_NONE && enemy_attacking==ENEMY_NONE) {
                        obj_enemy[nenemy].obj_character.x = newx;
                        obj_enemy[nenemy].obj_character.y = newy;
                        obj_enemy[nenemy].obj_character.animation=ANIM_WALK;
                        obj_enemy[nenemy].obj_character.flipH=(dx<0);
                        update_enemy(nenemy);
                        has_moved=true;
                    }
                if (has_moved==false && enemy_attacking!=nenemy) anim_enemy(nenemy,ANIM_IDLE);
                }
            }
        }
    }
}