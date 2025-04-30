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

extern bool is_combat_active; // Are we in a combat?

void start_combat(bool start); // Start (or end) a combat
void hit_enemy(u16 nenemy); // Hit an enemy
void hit_caracter(u16 nchar); // Hit a character
void show_or_hide_enemy_combat_interface(bool show); // Show enemy face and hitpoints in the interface
void combat_update(void); // Update all state machines in combat

#endif