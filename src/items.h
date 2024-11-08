#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "entity.h"

#define MAX_ITEMS 10
#define ITEM_NONE 254

typedef struct {
    Entity entity;
    bool interactable; // Can the player interact?
    u16 x_in_background; // X coordinate, relative to the background
} Item;

Item obj_item[MAX_ITEMS];
Sprite *spr_item[MAX_ITEMS];

u16 pending_item_interaction; // The user has interacted with an item

void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u8 collision_width, u8 collision_x_offset, u8 collision_height, u8 collision_y_offset); // Initialize an item
void release_item(u16 index); // Release an item
void display_item_if_visible(u16 nitem); // Hide or display an item depending if the X coordinate is visible in the screen
void check_items_visibility(void); // Check visibility of every item in screen
u16 detect_nearby_item(); // Detect if the active character would collide with an object in a nearby position

#endif
