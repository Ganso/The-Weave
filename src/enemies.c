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
    obj_enemy[numenemy].obj_character = (Entity) { nsprite, 0, 0, npal, false, false, ANIM_IDLE, false };

    spr_enemy[numenemy] = SPR_addSpriteSafe(nsprite, obj_enemy[numenemy].obj_character.x, obj_enemy[numenemy].obj_character.y, 
                                       TILE_ATTR(npal, obj_enemy[numenemy].obj_character.priority, false, obj_enemy[numenemy].obj_character.flipH));
    
    if (spr_enemy[numenemy] != NULL) {
        SPR_setVisibility(spr_enemy[numenemy], HIDDEN);
    }
}