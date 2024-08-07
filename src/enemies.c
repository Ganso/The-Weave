#include <genesis.h>
#include "globals.h"

// Initialize enemy pattern
void init_enemy_patterns(void)
{
    obj_Pattern_Enemy[PTRN_EN_ELECTIC]=(Pattern_Enemy) {4, {1,2,3,4}};
    obj_Pattern_Enemy[PTRN_EN_BITE]=(Pattern_Enemy) {2, {2,3,NULL,NULL}};
}

// initialize enemy classes
void init_enemy_classes(void)
{
    obj_enemy_class[ENEMY_CLS_BADBOBBIN]=(Enemy_Class) {10, {true, true}};
}

// Initialize an enemy
void init_enemy(u8 numenemy, u8 class)
{
    u8 npal = PAL3;
    u8 y_size = 96;
    const SpriteDefinition *nsprite = NULL;
    obj_enemy[numenemy].class=obj_enemy_class[class];
    obj_enemy[numenemy].hitpoints=obj_enemy_class[class].max_hitpoints;

    switch (class)
    {
    case ENEMY_CLS_BADBOBBIN:
        nsprite = &badbobbin_sprite;
        break;
    default:
        return; 
    }
    // * Sprite definition, x, y, palette, priority, flipH, animation, visible
    obj_enemy[numenemy].obj_character = (Entity) { nsprite, 0, 0, y_size, npal, false, false, ANIM_IDLE, false };

    spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, obj_enemy[numenemy].obj_character.x, obj_enemy[numenemy].obj_character.y, 
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    if (spr_enemy[numenemy] != NULL) {
        SPR_setVisibility(spr_enemy[numenemy], HIDDEN);
    }
}

// Release an enemy from memory (Just the sprite, keep the Enemy struct)
void release_enemy(u8 nenemy)
{
    if (spr_enemy[nenemy] != NULL)
    {
        SPR_releaseSprite(spr_enemy[nenemy]);
        spr_enemy[nenemy] = NULL;
    }
}

// Update an enemy based on every parameter
void update_enemy(u8 nenemy)
{
    SPR_setPosition(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.x, obj_enemy[nenemy].obj_character.y);
    SPR_setPriority(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.priority);
    SPR_setVisibility(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.visible ? VISIBLE : HIDDEN);
    SPR_setHFlip(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.flipH);
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
    SPR_update();
}

// Show or hide an enemy
void show_enemy(u8 nenemy, bool show)
{
    obj_enemy[nenemy].obj_character.visible = show;
    SPR_setVisibility(spr_enemy[nenemy], show ? VISIBLE : HIDDEN);
    SPR_update();
}

// Change an enemy's animation
void anim_enemy(u8 nenemy, u8 newanimation)
{
    obj_enemy[nenemy].obj_character.animation = newanimation;
    SPR_setAnim(spr_enemy[nenemy], obj_enemy[nenemy].obj_character.animation);
}

// Make an enemy look to the left (or right)
void look_enemy_left(u8 nenemy, bool direction_right)
{
    obj_enemy[nenemy].obj_character.flipH = direction_right;
    SPR_setHFlip(spr_enemy[nenemy], direction_right);
    SPR_update();
}

// Move an enemy to a new position
void move_enemy(u8 nenemy, s16 newx, s16 newy)
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
void move_enemy_instant(u8 nenemy, s16 x, s16 y)
{
    SPR_setPosition(spr_enemy[nenemy], x, y);
    obj_enemy[nenemy].obj_character.x = x;
    obj_enemy[nenemy].obj_character.y = y;
    update_bg();
}