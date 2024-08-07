#include <genesis.h>
#include "globals.h"

// Initialize a character
void init_character(u8 nchar)
{
    u8 npal = PAL1;
    u8 y_size = 64;
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
        default:
            return; 
        }
        // * Sprite definition, x, y, palette, priority, flipH, animation, visible
        obj_character[nchar] = (Entity) { nsprite, 0, 0, y_size, npal, false, false, ANIM_IDLE, false };
    } else {
        nsprite = obj_character[nchar].sd;
        npal = obj_character[nchar].palette;
    }

    spr_chr[nchar] = SPR_addSpriteSafe(nsprite, obj_character[nchar].x, obj_character[nchar].y, 
                                       TILE_ATTR(npal, obj_character[nchar].priority, false, obj_character[nchar].flipH));
    
    if (spr_chr[nchar] != NULL) {
        SPR_setVisibility(spr_chr[nchar], HIDDEN);
    }
}

// Release a character from memory (Just the sprite, keep the Entity)
void release_character(u8 nchar)
{
    if (spr_chr[nchar] != NULL)
    {
        SPR_releaseSprite(spr_chr[nchar]);
        spr_chr[nchar] = NULL;
    }
}

// Initialize a face
void init_face(u8 nface)
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
        default:
            return; // Salir si la cara no es válida
        }
        obj_face[nface] = (Entity) { nsprite, 0, 160, 64, npal, false, false, ANIM_IDLE, false };
    } else {
        nsprite = obj_face[nface].sd;
    }

    spr_face[nface] = SPR_addSpriteSafe(nsprite, obj_face[nface].x, obj_face[nface].y, 
                                        TILE_ATTR(obj_face[nface].palette, obj_face[nface].priority, false, obj_face[nface].flipH));
    
    if (spr_face[nface] != NULL) {
        SPR_setVisibility(spr_face[nface], HIDDEN);
        SPR_setDepth(spr_face[nface], SPR_MIN_DEPTH); // Faces are above any other sprite
    }
}

// Release a face from memory (Just the sprite, keep the Entity)
void release_face(u8 nface)
{
    if (spr_face[nface] != NULL)
    {
        SPR_releaseSprite(spr_face[nface]);
        spr_face[nface] = NULL;
    }
}

// Update a character based on every parameter
void update_character(u8 nchar)
{
    SPR_setPosition(spr_chr[nchar],obj_character[nchar].x,obj_character[nchar].y);
    SPR_setPriority(spr_chr[nchar],obj_character[nchar].priority);
    SPR_setVisibility(spr_chr[nchar],obj_character[nchar].visible?VISIBLE:HIDDEN);
    SPR_setHFlip(spr_chr[nchar],obj_character[nchar].flipH);
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
    SPR_update();
}

// Show or hide a character
void show_character(u8 nchar, bool show)
{
    obj_character[nchar].visible=show;
    SPR_setVisibility(spr_chr[nchar],show?VISIBLE:HIDDEN);
    SPR_update();
}

// Change a character's animation
void anim_character(u8 nchar, u8 newanimation)
{
    obj_character[nchar].animation=newanimation;
    SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
}

// Make a character look to the left (or right)
void look_left(u8 nchar, bool direction_right)
{
    obj_character[nchar].flipH=direction_right;
    SPR_setHFlip (spr_chr[nchar], direction_right);
    SPR_update();
}

// Move a character to a new position
void move_character(u8 nchar, s16 newx, s16 newy)
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
void move_character_instant(u8 nchar,s16 x,s16 y)
{
    SPR_setPosition(spr_chr[nchar], x, y);
    obj_character[nchar].x = x;
    obj_character[nchar].y = y;
    update_bg();
}

// Update characters and enemies depth
void update_sprites_depth(void)
{
    u8 i;

    // Update character depth
    for (i = 0; i < MAX_CHR; i++) {
        if (spr_chr[i] != NULL) {
            SPR_setDepth(spr_chr[i], -obj_character[i].y-obj_character[i].y_size); // Negative of the bottom line of the sprite
        }
    }

    // Update enemies depth
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (spr_enemy[i] != NULL) {
            SPR_setDepth(spr_enemy[i], -obj_enemy[i].obj_character.y-obj_enemy[i].obj_character.y_size); // Negative of the bottom line of the sprite
        }
    }
}