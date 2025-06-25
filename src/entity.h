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

// Fixed point helpers (10.6 format)
#define FIX16_ONE       64
#define INT_TO_FIX16(n) ((fix16)((n) << 6))
#define FIX16_TO_INT(n) ((s16)((n) / FIX16_ONE))
#define FIX16_MUL(a,b)  ((fix16)(((s32)(a) * (s32)(b)) >> 6))

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
    s16                     x;
    s16                     y;
    fix16                   fx;
    fix16                   fy;
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
    fix16                   velocity;
    bool                    drops_shadow;
    u16                     modeTimer;
} Entity;

void move_entity(Entity *entity, Sprite *sprite, s16 newx, s16 newy); // Move an entity

#endif
