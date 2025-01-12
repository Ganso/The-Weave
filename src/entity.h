#ifndef _ENTITY_H_
#define _ENTITY_H_

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3

// Global variables
extern bool movement_active;

// Entities states
typedef enum {
    STATE_IDLE,
    STATE_WALKING,
    STATE_PLAYING_NOTE,
    STATE_PATTERN_FINISHED,
    STATE_PATTERN_CHECK,
    STATE_PATTERN_EFFECT,
    STATE_PATTERN_EFFECT_FINISH,
    STATE_ATTACK_FINISHED,
    STATE_FOLLOWING
} GameState;

// Game entity definition
typedef struct
{
    bool                    active;
    const SpriteDefinition  *sd;
    s16                     x;
    s16                     y;
    u8                      x_size;
    u8                      y_size;
    u16                     palette;
    u8                      priority;
    u8                      flipH;
    u8                      animation;
    bool                    visible;
    u8                      collision_x_offset;
    u8                      collision_y_offset;
    u8                      collision_width;
    u8                      collision_height;
    GameState               state;
    bool                    follows_character;
    u8                      follow_speed;
    bool                    drops_shadow;
} Entity;

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy); // Move an entity

#endif
