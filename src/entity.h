#ifndef _ENTITY_H_
#define _ENTITY_H_

// Animations
#define ANIM_IDLE       0
#define ANIM_WALK       1
#define ANIM_ACTION     2
#define ANIM_MAGIC      3
#define ANIM_HURT       4

// Global variables
extern bool movement_active;

typedef s32 fastfix32;                       // 16.16 fixed point value
#define FASTFIX32_FROM_INT(v) ((fastfix32)((v) << 16))
#define FASTFIX32_TO_INT(v)   ((s16)((v) >> 16))

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
    STATE_FOLLOWING,
    STATE_HIT,
} GameState;

// Game entity definition
typedef struct
{
    bool                    active;
    const SpriteDefinition  *sd;
    const SpriteDefinition  *sd_shadow;
    fastfix32               x;
    fastfix32               y;
    fastfix32               speed;
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
    bool                    drops_shadow;
    u16                     modeTimer;
} Entity;

void move_entity(Entity *entity, Sprite *sprite, fastfix32 newx, fastfix32 newy); // Move an entity

#endif
