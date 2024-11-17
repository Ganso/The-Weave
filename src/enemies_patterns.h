#ifndef _ENM_PATTERNS_H_
#define _ENM_PATTERNS_H_


// Enemy Patterns
#define MAX_PATTERN_ENEMY 2

#define PTRN_EN_NONE         254
#define PTRN_EN_ELECTIC      0   // Electricity spell
#define PTRN_EN_BITE         1   // Bite Spell

#define MAX_ATTACK_NOTE_PLAYING_TIME  500  // Attack note playing time (in milliseconds)
#define MAX_TIME_AFTER_ATTACK        1000  // Time to stay in STATE_ATTACK_FINISHED (in milliseconds)

// Pattern-specific max effect times (in milliseconds)
#define MAX_EFFECT_TIME_ELECTRIC 1600
#define MAX_EFFECT_TIME_BITE     1400

// Enemy pattern struct
typedef struct
{
    u8 numnotes;
    u8 notes[4];
    u16 recharge_time;
} Pattern_Enemy;
extern Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY]; // Enemie pattern object

// Enemy pattern state variables
extern u16 enemy_attacking; // Which enemy is attacking?
extern u16 enemy_attack_pattern; // Which pattern is the enemy using?
extern u8 enemy_attack_pattern_notes; // How many notes has the enemy launched yet?
extern u16 enemy_attack_time; // How long is the enemy attacking?
extern bool enemy_attack_effect_in_progress; // An enemy pattern attack effect is in progress
extern u16 enemy_attack_effect_time; // How long has been the enemy pattern effect working?
extern bool enemy_note_active[6]; // Is the note # MI-DO active?

// Initialization function
void init_enemy_patterns(void); // initialize enemy patterns

// Main state machine function
void check_enemy_state(void); // Main state machine for enemy pattern system

// Helper functions
void show_enemy_note(u8 nnote, bool visible, bool play); // Show and play (or not) an enemy note
void finish_enemy_pattern_effect(void); // Finish the enemy pattern effect
void cleanup_enemy_notes(void); //Clean up all active enemy notes

// Pattern-specific effect functions
void launch_electric_enemy_pattern(void);
void do_electric_enemy_pattern_effect(void);
void finish_electric_enemy_pattern_effect(void);

void launch_bite_enemy_pattern(void);
void do_bite_enemy_pattern_effect(void);
void finish_bite_enemy_pattern_effect(void);

#endif
