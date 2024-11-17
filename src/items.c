#include <genesis.h>
#include "globals.h"
#include "characters.h"

// Global variable definitions
Item obj_item[MAX_ITEMS];
Sprite *spr_item[MAX_ITEMS];
u16 pending_item_interaction;


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
    obj_item[nitem].entity = (Entity) { true, spritedef, 0, y, x_size, y_size, npal, false, false, ANIM_IDLE, true, collision_x_offset, collision_y_offset, collision_width, collision_height, STATE_IDLE, FALSE, 0 };
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
    s16 x=get_x_in_screen(obj_item[nitem].x_in_background, obj_item[nitem].entity.x_size);

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

// Detect if the active character is near an interactable item
u16 detect_nearby_item()
{
    u16 nitem;
    u16 min_distance = 0xFFFF; // Maximum possible distance
    u16 closest_item = ITEM_NONE;
    u16 distance;
    
    // Get active character's position
    u16 char_x = obj_character[active_character].x + 
                 obj_character[active_character].collision_x_offset + 
                 (obj_character[active_character].collision_width / 2);
    u8 char_y = obj_character[active_character].y + 
                obj_character[active_character].collision_y_offset + 
                (obj_character[active_character].collision_height / 2);

    // Check all active and visible items
    for (nitem = 0; nitem < MAX_ITEMS; nitem++) {
        if (obj_item[nitem].entity.active && obj_item[nitem].entity.visible) {
            distance = item_distance(nitem, char_x, char_y);
            KDebug_AlertNumber(distance);
            // If this item is closer than previous closest and within interaction distance
            if (distance < min_distance && distance <= MAX_INTERACTIVE_DISTANCE) {
                min_distance = distance;
                closest_item = nitem;
            }
        }
    }

    return closest_item;
}
