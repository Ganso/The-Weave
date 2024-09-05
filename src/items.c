#include <genesis.h>
#include "globals.h"
#include "characters.h"

// Initialize an item
void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u8 collision_width, u8 collision_x_offset, u8 collision_height, u8 collision_y_offset)
{
    u8 x_size, y_size;

    // Width and height (from the sprite defition)
    x_size=spritedef->w;
    y_size=spritedef->h;

    // Collision box (if set to 0, use default values)
    if (collision_width==0) collision_width=x_size/2; // Half width size
    if (collision_x_offset==0) collision_x_offset=x_size/4; // Centered in X
    if (collision_height==0) collision_height=2; // Two lines heght
    if (collision_y_offset==0) collision_y_offset=y_size-1; // At the feet

    obj_item[nitem].x_in_background=x_in_background;

    // We set X to 0, as we are gonna calc it later
    obj_item[nitem].entity = (Entity) { true, spritedef, 0, y, x_size, y_size, npal, false, false, ANIM_IDLE, true, collision_x_offset, collision_y_offset, collision_width, collision_height };
    spr_item[nitem] = SPR_addSpriteSafe(spritedef, 0, y, TILE_ATTR(npal, false, false, false));

    // Define real X in the sprite, and show it or hide it depending if it's visible in the background
    display_item_if_visible(nitem);
}

// Release an item
void release_item(u16 nitem)
{
    obj_item[nitem].entity.active = false;
    if (spr_item[nitem] != NULL)
    {
        SPR_releaseSprite(spr_item[nitem]);
        spr_item[nitem] = NULL;
    }
}

// Hide or display an item depending if the X coordinate is visible in the screen
void display_item_if_visible(u16 nitem)
{
    u16 x=get_x_in_screen(obj_item[nitem].x_in_background, obj_item[nitem].entity.x_size);

    if (x!=X_OUT_OF_BOUNDS) {
        obj_item[nitem].entity.x=x;
        obj_item[nitem].entity.visible=true;
        SPR_setPosition(spr_item[nitem], obj_item[nitem].entity.x, obj_item[nitem].entity.y);
        SPR_setVisibility(spr_item[nitem], VISIBLE);
    }
    else {
        obj_item[nitem].entity.visible=false;
        SPR_setVisibility(spr_item[nitem], HIDDEN);
    }
}

// Check visibility of every item in screen
void check_items_visibility(void)
{
    u16 nitem;

    for (nitem=0;nitem<MAX_ITEMS;nitem++) {
        if (obj_item[nitem].entity.active==true) {
            display_item_if_visible(nitem);
        }
    }
}

// Detect collisions between a character and every item, given some new coordinates
u16 detect_char_item_collision(u16 nchar, u16 x, u8 y)
{
    u16 nitem;
    u16 char_left, char_right, char_top, char_bottom;
    u16 item_left, item_right, item_top, item_bottom;

    // Calculate character's bounding box
    char_left = x + obj_character[nchar].collision_x_offset;
    char_right = char_left + obj_character[nchar].collision_width;
    char_top = y + obj_character[nchar].collision_y_offset;
    char_bottom = char_top + obj_character[nchar].collision_height;

    for (nitem = 0; nitem < MAX_ITEMS; nitem++)
    {
        if (obj_item[nitem].entity.active && obj_item[nitem].entity.visible)
        {
            // Calculate item's bounding box
            item_left = obj_item[nitem].entity.x + obj_item[nitem].entity.collision_x_offset;
            item_right = item_left + obj_item[nitem].entity.collision_width;
            item_top = obj_item[nitem].entity.y + obj_item[nitem].entity.collision_y_offset;
            item_bottom = item_top + obj_item[nitem].entity.collision_height;

            // Check for collision
            if (char_left < item_right && char_right > item_left &&
                char_top < item_bottom && char_bottom > item_top)
            {
                return nitem; // Return the index of the collided item
            }
        }
    }

    return ITEM_NONE; // No collision detected
}

// Detect if the active character would collide with an object in a nearby position
u16 detect_nearby_item()
{
    u16 x=obj_character[active_character].x;
    u8 y=obj_character[active_character].y;
    u16 nitem;

    nitem=detect_char_item_collision(active_character,x-1,y);
    if (nitem!=ITEM_NONE) return(nitem);

    nitem=detect_char_item_collision(active_character,x+1,y);
    if (nitem!=ITEM_NONE) return(nitem);

    nitem=detect_char_item_collision(active_character,x,y-1);
    if (nitem!=ITEM_NONE) return(nitem);

    nitem=detect_char_item_collision(active_character,x,y+1);
    return(nitem);
}