#ifndef _ENEMIES_H_
#define _ENEMIES_H_

#include "globals.h"

// Enemy class IDs
#define ENEMY_CLS_BADBOBBIN    0
#define ENEMY_CLS_3HEADMONKEY  1

/**
 * @brief Enemy class definition structure
 */
typedef struct {
    u16 max_hitpoints;          // Maximum HP for this class
    bool has_pattern[MAX_PATTERN_ENEMY]; // Available patterns
    bool follows_character;     // Whether enemy follows player
    u8 follow_speed;           // Movement speed when following
} Enemy_Class;

// Enemy class definitions
extern Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES];

/**
 * @brief Enemy instance structure
 */
typedef struct {
    Enemy_Class class;          // Reference to class definition
    u16 class_id;              // Class type
    Entity obj_character;       // Base entity data
    u16 hitpoints;             // Current HP
    u16 last_pattern_time[MAX_PATTERN_ENEMY]; // Pattern cooldowns
} Enemy;

// Enemy instances and sprites
extern Enemy obj_enemy[MAX_ENEMIES];
extern Sprite *spr_enemy[MAX_ENEMIES];
extern Sprite *spr_enemy_face[MAX_ENEMIES];
extern Sprite *spr_enemy_shadow[MAX_ENEMIES];

/**
 * @brief Initialize enemy class definitions
 */
void init_enemy_classes(void);

/**
 * @brief Initialize a new enemy instance
 * @param numenemy Enemy slot index
 * @param class Enemy class ID
 */
void init_enemy(u16 numenemy, u16 class);

/**
 * @brief Release enemy resources
 * @param nenemy Enemy to release
 */
void release_enemy(u16 nenemy);

/**
 * @brief Update enemy sprite properties
 * @param nenemy Enemy to update
 */
void update_enemy(u16 nenemy);

/**
 * @brief Show/hide enemy and shadow
 * @param nenemy Enemy to show/hide
 * @param show Whether to show
 */
void show_enemy(u16 nenemy, bool show);

/**
 * @brief Set enemy animation
 * @param nenemy Enemy to animate
 * @param newanimation Animation ID
 */
void anim_enemy(u16 nenemy, u8 newanimation);

/**
 * @brief Set enemy facing direction
 * @param nenemy Enemy to update
 * @param direction_right true to face right
 */
void look_enemy_left(u16 nenemy, bool direction_right);

/**
 * @brief Move enemy with animation
 * @param nenemy Enemy to move
 * @param newx New X position
 * @param newy New Y position
 */
void move_enemy(u16 nenemy, s16 newx, s16 newy);

/**
 * @brief Move enemy instantly
 * @param nenemy Enemy to move
 * @param x New X position
 * @param y New Y position
 */
void move_enemy_instant(u16 nenemy, s16 x, s16 y);

/**
 * @brief Update enemy positions to follow player
 */
void approach_enemies(void);

/**
 * @brief Update enemy shadow position
 * @param nenemy Enemy to update
 */
void update_enemy_shadow(u16 nenemy);

#endif // _ENEMIES_H_
