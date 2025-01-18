#ifndef _COMBAT_H_
#define _COMBAT_H_

#include "globals.h"
#include "combat_states.h"

/**
 * Combat System
 * 
 * This module handles the core combat mechanics including:
 * - Combat initialization and cleanup
 * - Damage calculation and application
 * - Combat UI management (life counters, enemy faces, note indicators)
 * - State tracking for active combats using a state machine
 * 
 * The combat system uses a state machine to manage combat flow:
 * - IDLE: No patterns being cast
 *   - Transitions to PLAYER_CAST when player starts pattern
 *   - Transitions to ENEMY_CAST when enemy starts pattern
 * 
 * - PLAYER_CAST: Player is casting a pattern
 *   - Tracks note sequence and timing
 *   - Can be interrupted by enemy patterns
 *   - Transitions to PATTERN_EFFECT on completion
 * 
 * - ENEMY_CAST: Enemy is casting a pattern
 *   - Plays enemy note sequence
 *   - Can be interrupted by player patterns
 *   - Transitions to PATTERN_EFFECT on completion
 * 
 * - PATTERN_EFFECT: Pattern effect is being applied
 *   - Handles pattern-specific effects and durations
 *   - Can be interrupted by opposing patterns
 *   - Returns to IDLE when effect completes
 * 
 * - END: Combat is ending
 *   - Cleans up combat state
 *   - Returns game to exploration mode
 * 
 * State transitions occur through messages:
 * - MSG_COMBAT_START: Start combat with an enemy
 * - MSG_PATTERN_COMPLETE: Pattern cast completed
 * - MSG_ENEMY_ATTACK: Enemy starts casting
 * - MSG_ENEMY_HIT: Enemy takes damage
 * - MSG_PLAYER_HIT: Player takes damage
 * - MSG_COMBAT_END: Combat is ending
 * 
 * Pattern casting follows these rules:
 * 1. Only one pattern can be cast at a time (player or enemy)
 * 2. Players can interrupt enemy patterns and vice versa
 * 3. Interrupted patterns do not take effect
 * 4. Pattern effects have a duration and can be interrupted
 * 
 * Debug output is controlled by DEBUG_STATE_MACHINES:
 * - State transitions are logged
 * - Pattern casting progress is tracked
 * - Interruptions are reported
 * - Effect durations are monitored
 */

/**
 * @brief Start or end a combat sequence
 * @param start true to start combat, false to end it
 * 
 * When starting combat:
 * - Initializes combat state machine if needed
 * - Resets enemy states and HP
 * - Sets up combat UI elements
 * - Randomizes enemy pattern cooldowns
 * - Transitions to IDLE state
 * 
 * When ending combat:
 * - Cleans up combat state machine
 * - Removes UI elements
 * - Resets player movement state
 * - Transitions through END state
 */
void start_combat(bool start);

/**
 * @brief Apply damage to an enemy
 * @param nenemy Enemy index to damage
 * 
 * - Plays hit sound effect
 * - Interrupts enemy pattern if they were casting
 * - Updates enemy HP and UI
 * - Handles enemy defeat if HP reaches 0
 * - Sends MSG_ENEMY_HIT message
 * - May trigger combat end if enemy defeated
 */
void hit_enemy(u16 nenemy);

/**
 * @brief Apply damage to a player character
 * @param nchar Character index to damage
 * 
 * - Plays hurt sound effect
 * - Interrupts player pattern if casting
 * - Updates character state
 * - Sends MSG_PLAYER_HIT message
 * - May trigger pattern interruption
 */
void hit_character(u16 nchar);

/**
 * @brief Show or hide combat interface elements
 * @param show true to show interface, false to hide it
 * 
 * Controls visibility of:
 * - Enemy face sprites
 * - Life counter
 * - Note indicators
 * - Pattern effect indicators
 * 
 * Called during:
 * - Combat start/end
 * - Pattern casting
 * - Effect visualization
 */
void show_or_hide_enemy_combat_interface(bool show);

#endif // _COMBAT_H_
