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
typedef s32 fix32;

#define FIX32_ONE (1 << 16)

static inline fix32 to_fix32(s16 value)
{
    return ((fix32)value) << 16;
}

static inline s16 to_int(fix32 value)
{
    return (s16)(value >> 16);
}

typedef struct
{
    bool                    active;
    const SpriteDefinition  *sd;
    const SpriteDefinition  *sd_shadow;
    fix32                   x;
    fix32                   y;
    u8                      x_size;
    u8                      y_size;
    fix32                   speed;
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

void move_entity(Entity *entity, Sprite *sprite, fix32 newx, fix32 newy); // Move an entity using fixed point values

#endif
