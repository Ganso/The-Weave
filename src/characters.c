#include <genesis.h>
#include "globals.h"

// Initialize a character
void init_character(u16 nchar)
{
    u8 npal = PAL1;
    u8 x_size, y_size; // We can get them from the Sprite Definition
    u8 collision_x_offset=4;
    u8 collision_y_offset=63;
    u8 collision_width=36;
    u8 collision_height=2;
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
        x_size=nsprite->w; // Get width and height from the Sprite Definition
        y_size=nsprite->h;
        obj_character[nchar] = (Entity) { true, nsprite, 0, 0, x_size, y_size, npal, false, false, ANIM_IDLE, false, collision_x_offset, collision_y_offset, collision_width, collision_height };
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
        default:
            return;
        }
        obj_face[nface] = (Entity) { true, nsprite, 0, 160, 64, 64, npal, false, false, ANIM_IDLE, false, 0, 0, 0, 0 };
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
}

// Show or hide a character
void show_character(u16 nchar, bool show)
{
    obj_character[nchar].visible=show;
    SPR_setVisibility(spr_chr[nchar],show?VISIBLE:HIDDEN);
    SPR_update();
}

// Change a character's animation
void anim_character(u16 nchar, u8 newanimation)
{
    if (obj_character[nchar].animation!=newanimation) {
        obj_character[nchar].animation=newanimation;
        SPR_setAnim(spr_chr[nchar],obj_character[nchar].animation);
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
            SPR_setDepth(spr_item[i], -obj_item[i].entity.y-obj_item[i].entity.y_size); // Negative of the bottom line of the sprite
        }
    }

    // Update enemies depth
    for (i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active==true) {
            SPR_setDepth(spr_enemy[i], -obj_enemy[i].obj_character.y-obj_enemy[i].obj_character.y_size); // Negative of the bottom line of the sprite
        }
    }
}