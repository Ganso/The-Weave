#ifndef _ENEMIES_H_
#define _ENEMIES_H_

// Enemy Patterns
#define MAX_PATTERN_ENEMY 2
#define PTRN_EN_NONE         254
#define PTRN_EN_ELECTIC      0   // Electricity spell
#define PTRN_EN_BITE         1   // Bite Spell

typedef struct
{
    u8 numnotes;
    u8 notes[4];
    u16 recharge_time;
} Pattern_Enemy;
Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY]; // Enemie pattern object

// Enemy classes
#define MAX_ENEMY_CLASSES 10
#define ENEMY_CLS_BADBOBBIN    0
#define ENEMY_CLS_3HEADMONKEY  1

typedef struct
{
    u16 max_hitpoints;
    bool has_pattern[MAX_PATTERN_ENEMY];
    bool follows_character;
    u8 follow_speed;
} Enemy_Class;
Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES]; // Enemy class object

// Enemies
#define MAX_ENEMIES 10
#define ENEMY_NONE 254

typedef struct
{
    Enemy_Class class;
    u16 class_id;
    Entity obj_character;
    u16 hitpoints;
    u16 last_pattern_time[MAX_PATTERN_ENEMY];
} Enemy;
Enemy obj_enemy[MAX_ENEMIES]; // Enemy object
Sprite *spr_enemy[MAX_ENEMIES]; // Enemy sprites
Sprite *spr_enemy_face[MAX_ENEMIES]; // Enemy faces sprites

void init_enemy_patterns(void); // initialize enemy patterns
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

#endif
