#ifndef _ENEMIES_H_
#define _ENEMIES_H_

// Patterns
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

// Enemies

#define MAX_ENEMIES 1

typedef struct
{
    u16 hitpoints;
    bool has_pattern[MAX_PATTERN_ENEMY];
} Enemy;

Entity obj_character_enemy[MAX_ENEMIES]; Enemy character object
Enemy obj_enemy[MAX_ENEMIES]; // Enemy object


void init_enemy_patterns(u8 num_enemy_pattern); // Initialize enemy pattern

#endif