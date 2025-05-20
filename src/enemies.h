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
// ******** Those will substitue the old enemy state-machine **********
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
extern Sprite *spr_enemy_face[MAX_ENEMIES]; // Enemy faces sprites
extern Sprite *spr_enemy_shadow[MAX_ENEMIES]; // Enemy shadows sprites

void init_enemy_classes(void); // initialize enemy classes
void init_enemy(u16 numenemy, u16 class); // Initialize an enemy
void release_enemy(u16 nenemy); // Release an enemy from memory
void update_enemy(u16 nenemy); // Update an enemy based on every parameter
void show_enemy(u16 nenemy, bool show); // Show or hide an enemy
void anim_enemy(u16 nenemy, u8 newanimation); // Change an enemy's animation
void look_enemy_left(u16 nenemy, bool direction_right); // Make an enemy look to the left (or right)
void move_enemy(u16 nenemy, s16 newx, s16 newy); // Move an enemy to a new position
void move_enemy_instant(u16 nenemy, s16 x, s16 y); // Move an enemy to a new position (instantly)
void approach_enemies(void); // Approach enemies to active character
void update_enemy_shadow(u16 nenemy); // Update shadow position for an enemy

#endif
