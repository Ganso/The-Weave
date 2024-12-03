#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "entity.h"

#define MAX_ITEMS 10
#define ITEM_NONE 254
#define COLLISION_DEFAULT 9999

typedef struct {
    Entity entity;
    //bool interactable; // Can the player interact?
    u16 x_in_background; // X coordinate, relative to the background
    bool is_background;
} Item;

extern Item obj_item[MAX_ITEMS];
extern Sprite *spr_item[MAX_ITEMS];

extern u16 pending_item_interaction; // The user has interacted with an item

void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u16 collision_width, u16 collision_x_offset, u16 collision_height, u16 collision_y_offset, bool is_background); // Initialize an item
void release_item(u16 nitem); // Release an item
void display_item_if_visible(u16 nitem); // Hide or display an item depending if the X coordinate is visible in the screen
void check_items_visibility(void); // Check visibility of every item in screen
u16 detect_nearby_item(); // Detect if the active character would collide with an object in a nearby position

#endif
