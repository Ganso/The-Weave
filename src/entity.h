#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <genesis.h>
#include "game_constants.h"

// Global variables
extern bool movement_active;

/**
 * @brief Game entity definition
 */
typedef struct {
    bool                    active;             // Whether entity is active
    const SpriteDefinition  *sd;                // Sprite definition
    const SpriteDefinition  *sd_shadow;         // Shadow sprite definition
    s16                     x;                  // X position
    s16                     y;                  // Y position
    u8                      x_size;             // Width
    u8                      y_size;             // Height
    u16                     palette;            // Color palette
    u8                      priority;           // Sprite priority
    u8                      flipH;              // Horizontal flip
    u8                      animation;          // Current animation
    bool                    visible;            // Visibility flag
    u8                      collision_x_offset; // Collision box X offset
    u8                      collision_y_offset; // Collision box Y offset
    u8                      collision_width;    // Collision box width
    u8                      collision_height;   // Collision box height
    u8                      state;              // Current state
    bool                    follows_character;  // Whether entity follows player
    u8                      follow_speed;       // Following movement speed
    bool                    drops_shadow;       // Whether entity casts shadow
} Entity;

/**
 * @brief Move entity to new position
 * @param entity Entity to move
 * @param sprite Entity's sprite
 * @param newx New X position
 * @param newy New Y position
 */
void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy);

#endif // _ENTITY_H_
