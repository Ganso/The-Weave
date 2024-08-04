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
} Pattern_Enemy;
Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY]; // Enemie pattern object

// Enemy classes
#define MAX_ENEMY_CLASSES 10
#define ENEMY_CLS_BADBOBBIN 0

typedef struct
{
    u16 max_hitpoints;
    bool has_pattern[MAX_PATTERN_ENEMY];
} Enemy_Class;
Enemy_Class obj_enemy_class[MAX_ENEMY_CLASSES]; // Enemy class object

// Enemies
#define MAX_ENEMIES 10

typedef struct
{
    Enemy_Class class;
    Entity obj_character;
    u16 hitpoints;
} Enemy;
Enemy obj_enemy[MAX_ENEMIES]; // Enemy object
Sprite *spr_enemy[MAX_ENEMIES]; // Enemy sprites

void init_enemy_patterns(void); // initialize enemy patterns
void init_enemy_classes(void); // initialize enemy classes
void init_enemy(u8 numenemy, u8 class); // Initialize an enemy

#endif