#include <genesis.h>
#include "globals.h"
#include "characters.h"

Item obj_item[MAX_ITEMS];              // Array of game items with their properties
Sprite *spr_item[MAX_ITEMS];           // Array of item sprites
u16 pending_item_interaction;          // ID of item currently pending interaction

void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u16 collision_width, u16 collision_x_offset, u16 collision_height, u16 collision_y_offset, u8 check_depth)    // Initialize item with sprite and collision properties
{
    u8 x_size, y_size;

    if (spritedef == NULL) {
        return;
    }

    // Width and height (from the sprite defition)
    x_size=spritedef->w;
    y_size=spritedef->h;

    // Collision box (if set to COLLISION_DEFAULT, use default values)
    if (collision_width==COLLISION_DEFAULT) collision_width=x_size/2; // Half width size
    if (collision_x_offset==COLLISION_DEFAULT) collision_x_offset=x_size/4; // Centered in X
    if (collision_height==COLLISION_DEFAULT) collision_height=2; // Two lines heght
    if (collision_y_offset==COLLISION_DEFAULT) collision_y_offset=y_size-1; // At the feet

    obj_item[nitem].x_in_background=x_in_background;
    obj_item[nitem].check_depth=check_depth;

    // We set X to 0, as we are gonna calc it later
    obj_item[nitem].entity = (Entity) { true, spritedef, NULL, 0, y, x_size, y_size, npal, false, false, ANIM_IDLE, true, collision_x_offset, collision_y_offset, collision_width, collision_height, STATE_IDLE, FALSE, 0, false };
    spr_item[nitem] = NULL;

    // Check visibility and load sprite if needed
    display_item_if_visible(nitem);
}

void release_item(u16 nitem)    // Free item resources and remove from game
{
    if (nitem >= MAX_ITEMS) {
        return;
    }
    
    // First hide sprite before releasing
    if (spr_item[nitem] != NULL) {
        SPR_setPosition(spr_item[nitem], -128, -128);
        SPR_setVisibility(spr_item[nitem], HIDDEN);
        SPR_update();  // Ensure sprite is moved before release
        SPR_releaseSprite(spr_item[nitem]);
        spr_item[nitem] = NULL;
    }
    
    obj_item[nitem].entity.active = false;
    obj_item[nitem].entity.visible = false;
}

void display_item_if_visible(u16 nitem)    // Show/hide item based on screen visibility
{
    if (nitem >= MAX_ITEMS || !obj_item[nitem].entity.active || obj_item[nitem].entity.sd == NULL) {
        return;
    }

    // Get sprite width from definition for visibility check
    u8 sprite_width = obj_item[nitem].entity.sd->w;
    
    // Calculate screen position
    s16 x = get_x_in_screen(obj_item[nitem].x_in_background, sprite_width);
    bool was_visible = obj_item[nitem].entity.visible;
    bool should_be_visible = (x != X_OUT_OF_BOUNDS);

    if (should_be_visible) {
        // Item should be visible
        if (spr_item[nitem] == NULL) {
            kprintf("Item %d now visible. LOADING.", nitem);
            spr_item[nitem] = SPR_addSpriteSafe(obj_item[nitem].entity.sd, 
                                               x, 
                                               obj_item[nitem].entity.y, 
                                               TILE_ATTR(obj_item[nitem].entity.palette, 
                                                       false, false, false));
            
            if (spr_item[nitem] == NULL) {
                obj_item[nitem].entity.visible = false;
                return;
            }
        }
        
        // Update position and ensure visibility
        s16 clamped_x = x;
        if (clamped_x < -(s16)(sprite_width - 1)) clamped_x = -(s16)(sprite_width - 1);
        if (clamped_x > SCREEN_WIDTH - 1) clamped_x = SCREEN_WIDTH - 1;
        
        SPR_setPosition(spr_item[nitem], clamped_x, obj_item[nitem].entity.y);
        SPR_setVisibility(spr_item[nitem], VISIBLE);
    } else {
        // Item should be invisible
        if (spr_item[nitem] != NULL) {
            kprintf("Item %d now invisible. UNLOADING.", nitem);
            SPR_setPosition(spr_item[nitem], -128, -128);
            SPR_setVisibility(spr_item[nitem], HIDDEN);
            SPR_update();  // Ensure sprite is moved before release
            SPR_releaseSprite(spr_item[nitem]);
            spr_item[nitem] = NULL;
        }
    }
    
    // Update final state
    obj_item[nitem].entity.visible = should_be_visible;
    obj_item[nitem].entity.x = x;
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
