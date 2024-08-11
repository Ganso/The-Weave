#ifndef _COMBAT_H_
#define _COMBAT_H_

#define MAX_ATTACK_NOTE_PLAYING_TIME  30  // Attack note playing time (.5 seconds)

bool is_combat_active; // Are we in a combat?
u16 enemy_attacking; // Which enemy is attacking?
u16 enemy_attack_pattern; // Which pattern is the enemy using?
u8 enemy_attack_pattern_notes; // How many notes has the enemy lauched yet?
u16 enemy_attack_time; // How long is the enemy attacking?
bool attack_effect_in_progress; // An enemy pattern attack effect is in progress
u16 attack_effect_time; // How long has been the enemy pattern effect working?

void start_combat(bool start); // Start (or end) a combat
void check_enemy_pattern(void); // Check if an enemy is going to launch a pattern
void enemy_launch_pattern(u8 numenemy, u8 npattern); // Enemy lauches a pattern
void enemy_launch_pattern_note(); // Enemy launches a pattern note
void show_enemy_note(u8 nnote, bool visible); // Show or hide notes
void do_enemy_pattern_effect(void); // An ennemy pattern is in progress. Make something happern.

#endif