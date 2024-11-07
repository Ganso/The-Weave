#ifndef _ENM_PATTERNS_H_
#define _ENM_PATTERNS_H_

#define MAX_ATTACK_NOTE_PLAYING_TIME  500  // Attack note playing time (in milliseconds)
#define MAX_TIME_AFTER_ATTACK        1000  // Time to stay in STATE_ATTACK_FINISHED (in milliseconds)

// Pattern-specific max effect times (in milliseconds)
#define MAX_EFFECT_TIME_ELECTRIC 1600
#define MAX_EFFECT_TIME_BITE     1400

// Enemy pattern state variables
u16 enemy_attacking; // Which enemy is attacking?
u16 enemy_attack_pattern; // Which pattern is the enemy using?
u8 enemy_attack_pattern_notes; // How many notes has the enemy launched yet?
u16 enemy_attack_time; // How long is the enemy attacking?
bool enemy_attack_effect_in_progress; // An enemy pattern attack effect is in progress
u16 enemy_attack_effect_time; // How long has been the enemy pattern effect working?
bool enemy_note_active[6]; // Is the note # MI-DO active?

// Main state machine function
void check_enemy_state(void); // Main state machine for enemy pattern system

// Helper functions
void show_enemy_note(u8 nnote, bool visible, bool play); // Show and play (or not) an enemy note
void finish_enemy_pattern_effect(void); // Finish the enemy pattern effect

// Pattern-specific effect functions
void launch_electric_pattern(void);
void do_electric_pattern_effect(void);
void finish_electric_pattern_effect(void);

void launch_bite_pattern(void);
void do_bite_pattern_effect(void);
void finish_bite_pattern_effect(void);

#endif
