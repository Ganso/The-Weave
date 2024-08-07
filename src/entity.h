#ifndef _ENTITY_H_
#define _ENTITY_H_

// Game entity definition
typedef struct
{
    const SpriteDefinition   *sd;
    s16                      x;
    s16                      y;
    u16                      palette;
    u8                       priority;
    u8                       flipH;
    u8                       animation;
    bool                     visible;
} Entity;

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy); // Mode an entity

#endif