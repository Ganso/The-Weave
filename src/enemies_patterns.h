#ifndef _ENM_PATTERNS_H_
#define _ENM_PATTERNS_H_

#define MAX_ATTACK_NOTE_PLAYING_TIME  500  // Attack note playing time (in milliseconds)

// Pattern-specific max effect times (in milliseconds)
#define MAX_EFFECT_TIME_ELECTRIC 1600
#define MAX_EFFECT_TIME_BITE     1400
u16 enemy_attacking; // Which enemy is attacking?
u16 enemy_attack_pattern; // Which pattern is the enemy using?
u8 enemy_attack_pattern_notes; // How many notes has the enemy lauched yet?
u16 enemy_attack_time; // How long is the enemy attacking?
bool enemy_attack_effect_in_progress; // An enemy pattern attack effect is in progress
u16 enemy_attack_effect_time; // How long has been the enemy pattern effect working?
bool enemy_note_active[6]; // Is the note # MI-DO active?

u16 enemy_attacking; // Which enemy is attacking?
u16 enemy_attack_pattern; // Which pattern is the enemy using?
u8 enemy_attack_pattern_notes; // How many notes has the enemy lauched yet?
u16 enemy_attack_time; // How long is the enemy attacking?
bool enemy_attack_effect_in_progress; // An enemy pattern attack effect is in progress
u16 enemy_attack_effect_time; // How long has been the enemy pattern effect working?
bool enemy_note_active[6]; // Is the note # MI-DO active?

void check_enemy_pattern(void); // Check if an enemy is going to launch a pattern
void enemy_launch_pattern(u8 numenemy, u8 npattern); // Enemy lauches a pattern
void enemy_launch_pattern_note(); // Enemy launches a pattern note
void show_enemy_note(u8 nnote, bool visible, bool play); // Show and play (or not) an enemy note
void do_enemy_pattern_effect(void); // An ennemy pattern is in progress. Make something happern.
void finish_enemy_pattern_effect(void); // Finish the enemy pattern effect
void handle_ongoing_attack(void); // Handle ongoing attack

// Pattern-specific functions
void check_electric_pattern(u8 numenemy, u8 npattern);
void launch_electric_pattern(void);
void do_electric_pattern_effect(void);
void finish_electric_pattern_effect(void);

void check_bite_pattern(u8 numenemy, u8 npattern);
void launch_bite_pattern(void);
void do_bite_pattern_effect(void);
void finish_bite_pattern_effect(void);

#endif