#ifndef _ITEMS_H_
#define _ITEMS_H_

#include "globals.h"

/**
 * @brief Item structure
 * Represents an interactive game item
 */
typedef struct {
    Entity entity;              // Base entity properties
    u16 x_in_background;       // X position relative to background
    bool is_background;        // Whether item is part of background
} Item;

// Item instances and sprites
extern Item obj_item[MAX_ITEMS];
extern Sprite *spr_item[MAX_ITEMS];

// Interaction tracking
extern u16 pending_item_interaction;  // Item player has interacted with

/**
 * @brief Initialize a new item
 * @param nitem Item slot index
 * @param spritedef Sprite definition
 * @param npal Palette index
 * @param x_in_background X position in background coordinates
 * @param y Y position
 * @param collision_width Collision box width
 * @param collision_x_offset Collision box X offset
 * @param collision_height Collision box height
 * @param collision_y_offset Collision box Y offset
 * @param is_background Whether item is part of background
 */
void init_item(u16 nitem, 
              const SpriteDefinition *spritedef,
              u8 npal,
              u16 x_in_background,
              u8 y,
              u16 collision_width,
              u16 collision_x_offset,
              u16 collision_height,
              u16 collision_y_offset,
              bool is_background);

/**
 * @brief Release item resources
 * @param nitem Item to release
 */
void release_item(u16 nitem);

/**
 * @brief Update item visibility based on screen position
 * @param nitem Item to update
 */
void display_item_if_visible(u16 nitem);

/**
 * @brief Update visibility of all items
 */
void check_items_visibility(void);

/**
 * @brief Check for nearby interactive items
 * @return Item ID if found, ITEM_NONE if none
 */
u16 detect_nearby_item(void);

#endif // _ITEMS_H_
