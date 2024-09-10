#ifndef _COMBAT_H_
#define _COMBAT_H_

/*
 * Combat System - Enemy Spell Casting
 * 
 * This module handles the enemy spell casting system in combat. The system is 
 * organized in a hierarchical structure with the following main components:
 * 
 * 1. Main Control Function:
 *    - check_enemy_pattern(): Oversees the entire spell casting process.
 * 
 * 2. Pattern Check Functions:
 *    - check_[pattern_name]_pattern(): Checks if conditions are met for a specific pattern.
 *    - Naming convention: check_[pattern_name]_pattern
 *    - Example: check_electric_pattern, check_bite_pattern
 * 
 * 3. Pattern Launch Functions:
 *    - launch_[pattern_name]_pattern(): Initiates a specific pattern.
 *    - Naming convention: launch_[pattern_name]_pattern
 *    - Example: launch_electric_pattern, launch_bite_pattern
 * 
 * 4. Pattern Effect Functions:
 *    - do_[pattern_name]_pattern_effect(): Handles the ongoing effect of a pattern.
 *    - Naming convention: do_[pattern_name]_pattern_effect
 *    - Example: do_electric_pattern_effect, do_bite_pattern_effect
 * 
 * 5. Pattern Finish Functions:
 *    - finish_[pattern_name]_pattern_effect(): Concludes the effect of a pattern.
 *    - Naming convention: finish_[pattern_name]_pattern_effect
 *    - Example: finish_electric_pattern_effect, finish_bite_pattern_effect
 * 
 * Workflow:
 * 1. check_enemy_pattern() is called regularly to manage the spell casting process.
 * 2. It calls the appropriate check_[pattern_name]_pattern() functions.
 * 3. If conditions are met, enemy_launch_pattern() is called, which then calls 
 *    the appropriate launch_[pattern_name]_pattern() function.
 * 4. During the effect, do_enemy_pattern_effect() is called, which then calls 
 *    the appropriate do_[pattern_name]_pattern_effect() function.
 * 5. When the effect ends, finish_enemy_pattern_effect() is called, which then 
 *    calls the appropriate finish_[pattern_name]_pattern_effect() function.
 * 
 * To add a new pattern:
 * 1. Create new functions following the naming conventions above.
 * 2. Add the pattern to the relevant switch statements in the main control functions.
 * 3. Define any necessary constants (like MAX_EFFECT_TIME_[PATTERN_NAME]).
 * 4. Update this header with the new pattern information.
 */

#define MAX_ATTACK_NOTE_PLAYING_TIME  500  // Attack note playing time in milliseconds

// Pattern-specific max effect times (in milliseconds)
#define MAX_EFFECT_TIME_ELECTRIC 1600
#define MAX_EFFECT_TIME_BITE     1400

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
void hit_enemy(u16 nenemy); // Hit an enemy
void hit_caracter(u16 nchar); // Hit a character
void handle_ongoing_attack(void); // Handle ongoing attack
void finish_enemy_pattern_effect(void); // Finish the enemy pattern effect

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