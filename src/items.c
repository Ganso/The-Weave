#include <genesis.h>
#include "globals.h"
#include "characters.h"

Item obj_item[MAX_ITEMS];              // Array of game items with their properties
Sprite *spr_item[MAX_ITEMS];           // Array of item sprites
u16 pending_item_interaction;          // ID of item currently pending interaction


void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u16 collision_width, u16 collision_x_offset, u16 collision_height, u16 collision_y_offset, bool is_background)    // Initialize item with sprite and collision properties
{
    u8 x_size, y_size;

    // Width and height (from the sprite defition)
    x_size=spritedef->w;
    y_size=spritedef->h;

    // Collision box (if set to COLLISION_DEFAULT, use default values)
    if (collision_width==COLLISION_DEFAULT) collision_width=x_size/2; // Half width size
    if (collision_x_offset==COLLISION_DEFAULT) collision_x_offset=x_size/4; // Centered in X
    if (collision_height==COLLISION_DEFAULT) collision_height=2; // Two lines heght
    if (collision_y_offset==COLLISION_DEFAULT) collision_y_offset=y_size-1; // At the feet

    obj_item[nitem].x_in_background=x_in_background;
    obj_item[nitem].is_background=is_background;

    // We set X to 0, as we are gonna calc it later
    obj_item[nitem].entity = (Entity) { true, spritedef, NULL, 0, y, x_size, y_size, npal, false, false, ANIM_IDLE, true, collision_x_offset, collision_y_offset, collision_width, collision_height, STATE_IDLE, FALSE, 0, false };
    spr_item[nitem] = SPR_addSpriteSafe(spritedef, 0, y, TILE_ATTR(npal, false, false, false));

    // Define real X in the sprite, and show it or hide it depending if it's visible in the background
    display_item_if_visible(nitem);
}

void release_item(u16 nitem)    // Free item resources and remove from game
{
    obj_item[nitem].entity.active = false;
    if (spr_item[nitem] != NULL)
    {
        SPR_releaseSprite(spr_item[nitem]);
        spr_item[nitem] = NULL;
    }
}

void display_item_if_visible(u16 nitem)    // Show/hide item based on screen visibility
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

void check_items_visibility(void)    // Update visibility state of all active items
{
    u16 nitem;

    for (nitem=0;nitem<MAX_ITEMS;nitem++) {
        if (obj_item[nitem].entity.active==true) {
            display_item_if_visible(nitem);
        }
    }
}

u16 detect_nearby_item()    // Find closest item within interaction range of active character
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
            // If this item is closer than previous closest and within interaction distance
            if (distance < min_distance && distance <= MAX_INTERACTIVE_DISTANCE) {
                min_distance = distance;
                closest_item = nitem;
            }
        }
    }

    return closest_item;
}
