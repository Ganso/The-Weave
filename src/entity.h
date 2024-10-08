#ifndef _ENTITY_H_
#define _ENTITY_H_

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3

// Game entity definition
typedef struct
{
    bool                     active;
    const SpriteDefinition   *sd;
    s16                      x;
    s16                      y;
    u8                       x_size;
    u8                       y_size;
    u16                      palette;
    u8                       priority;
    u8                       flipH;
    u8                       animation;
    bool                     visible;
    u8                       collision_x_offset;
    u8                       collision_y_offset;
    u8                       collision_width;
    u8                       collision_height;
} Entity;

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy); // Move an entity

#endif