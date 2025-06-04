#ifndef _ENEMIES_H_
#define _ENEMIES_H_

// Game constants
#define ENEMY_ENTITY_ID_BASE 100

// Enemies
#define MAX_ENEMIES 3
#define ENEMY_NONE 254
#define MAX_PATTERN_ENEMY 2

// Enemy classes
#define MAX_ENEMY_CLASSES 10
#define ENEMY_CLS_WEAVERGHOST    0
#define ENEMY_CLS_3HEADMONKEY  1

// Enemy modes
typedef enum {
    ENEMY_MODE_IDLE,        /* doing nothing */
    ENEMY_MODE_PLAY_NOTE,   /* playing the 4 pre-spell notes */
    ENEMY_MODE_CASTING,     /* spell effect is currently running */
    ENEMY_MODE_COOLDOWN     /* post-spell recovery */
} EnemyMode;

// Enemy classes
typedef struct
{
    u16 max_hitpoints;
    bool follows_character; // If true, the enemy will follow the character
    u8 follow_speed;
    bool has_pattern[MAX_PATTERN_ENEMY]; // If true, the enemy has a particular pattern
} Enemy_Class;
extern Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES]; // Enemy class object

// Enemies
typedef struct
{
    Enemy_Class class;
    u16 class_id;
    Entity obj_character;
    u16 hitpoints;
    u16 last_pattern_time[MAX_PATTERN_ENEMY]; // Last time a particular pattern was used (in frames)
    EnemyMode mode; // What's the enemy doing?
    u16 modeTimer; // Timer for the current mode (in frames)
} Enemy;


extern Enemy obj_enemy[MAX_ENEMIES]; // Enemy object
extern Sprite *spr_enemy[MAX_ENEMIES]; // Enemy sprites
extern Sprite *spr_enemy_shadow[MAX_ENEMIES]; // Enemy shadows sprites


/* --- Init / cleanup --- */
void init_enemy_classes(void);
void init_enemy(u16 numenemy, u16 class_id);
void release_enemy(u16 nenemy);

/* --- Runtime updates --- */
void update_enemy_shadow(u16 nenemy);
void update_enemy(u16 nenemy);
void show_enemy(u16 nenemy, bool show);
void anim_enemy(u16 nenemy, u8 newanimation);
void look_enemy_left(u16 nenemy, bool direction_right);
void move_enemy(u16 nenemy, s16 newx, s16 newy);
void move_enemy_instant(u16 nenemy, s16 x, s16 y);
void update_enemy_animations(void); // Update enemy animations based on their current state

/* --- AI movement --- */
void approach_enemies(void);

#endif
