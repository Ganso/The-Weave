#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "entity.h"

#define MAX_ITEMS 15
#define ITEM_NONE 254
#define COLLISION_DEFAULT 9999

// Depth check modes for items
#define FORCE_FOREGROUND 0
#define FORCE_BACKGROUND 1
#define CALCULATE_DEPTH 2

typedef struct {
    Entity entity;
    //bool interactable; // Can the player interact?
    u16 x_in_background; // X coordinate, relative to the background
    u8 check_depth; // How to handle sprite depth (FORCE_FOREGROUND, FORCE_BACKGROUND, or CALCULATE_DEPTH)
} Item;

extern Item obj_item[MAX_ITEMS];
extern Sprite *spr_item[MAX_ITEMS];

extern u16 last_interacted_item; // The item currently interacted by the player

void init_item(u16 nitem, const SpriteDefinition *spritedef, u8 npal, u16 x_in_background, u8 y, u16 collision_width, u16 collision_x_offset, u16 collision_height, u16 collision_y_offset, u8 check_depth); // Initialize an item
void release_item(u16 nitem); // Release an item
void display_item_if_visible(u16 nitem); // Hide or display an item deTODO if the X coordinate is visible in the screen
void check_items_visibility(void); // Check visibility of every item in screen
u16 detect_nearby_item(); // Detect if the active character would collide with an object in a nearby position

#endif
