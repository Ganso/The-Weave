#ifndef _COMBAT_STATES_H_
#define _COMBAT_STATES_H_

#include "globals.h"

/**
 * @brief Pattern cast data structure
 */
typedef struct {
    u16 caster_id;          // Who is casting
    u16 pattern_id;         // What pattern
    u16 notes_played;       // How many notes played
    u16 notes[6];          // Note sequence
    bool was_interrupted;   // If cast was interrupted
} PatternCastData;

/**
 * @brief Combat state machine data
 */
typedef struct CombatStateMachine {
    StateMachine base;           // Base state machine (must be first)
    bool is_active;             // If combat is active
    u16 active_enemy;           // Current enemy in combat
    PatternCastData player_cast; // Player pattern cast data
    PatternCastData enemy_cast;  // Enemy pattern cast data
    u16 effect_timer;           // Effect duration timer
    bool effect_in_progress;    // If effect is active
    u16 effect_source;          // Who caused the effect
    u16 effect_pattern;         // What pattern caused effect
} CombatStateMachine;

// Global combat state machine instance
extern CombatStateMachine combat_sm;

// Combat states
extern const State STATE_COMBAT_IDLE;           // No patterns active
extern const State STATE_COMBAT_PLAYER_CAST;    // Player is casting a pattern
extern const State STATE_COMBAT_ENEMY_CAST;     // Enemy is casting a pattern
extern const State STATE_COMBAT_PATTERN_EFFECT; // Pattern effect in progress
extern const State STATE_COMBAT_END;            // Combat ending

/**
 * @brief Initialize the combat state machine
 * @param csm Combat state machine to initialize
 * @param debug Enable debug output
 */
void combat_sm_init(CombatStateMachine* csm, bool debug);

/**
 * @brief Update combat state machine
 * @param csm Combat state machine to update
 */
void combat_sm_update(CombatStateMachine* csm);

/**
 * @brief Start combat with a new enemy
 * @param csm Combat state machine
 * @param enemy_id ID of enemy to fight
 */
void combat_sm_start(CombatStateMachine* csm, u16 enemy_id);

/**
 * @brief End current combat
 * @param csm Combat state machine
 */
void combat_sm_end(CombatStateMachine* csm);

/**
 * @brief Start player pattern casting
 * @param csm Combat state machine
 * @param player_id ID of casting player
 * @param pattern_id ID of pattern to cast
 * @return true if pattern casting started, false if another cast in progress
 */
bool combat_sm_player_cast_start(CombatStateMachine* csm, u16 player_id, u16 pattern_id);

/**
 * @brief Add note to player's pattern sequence
 * @param csm Combat state machine
 * @param note_id ID of note played
 * @return true if note was added, false if not in casting state
 */
bool combat_sm_player_cast_note(CombatStateMachine* csm, u16 note_id);

/**
 * @brief Start enemy pattern casting
 * @param csm Combat state machine
 * @param enemy_id ID of casting enemy
 * @param pattern_id ID of pattern to cast
 * @return true if pattern casting started, false if another cast in progress
 */
bool combat_sm_enemy_cast_start(CombatStateMachine* csm, u16 enemy_id, u16 pattern_id);

/**
 * @brief Add note to enemy's pattern sequence
 * @param csm Combat state machine
 * @param note_id ID of note played
 * @return true if note was added, false if not in casting state
 */
bool combat_sm_enemy_cast_note(CombatStateMachine* csm, u16 note_id);

/**
 * @brief Interrupt current pattern casting
 * @param csm Combat state machine
 * @param source_id ID of who caused interruption
 */
void combat_sm_interrupt_cast(CombatStateMachine* csm, u16 source_id);

#endif // _COMBAT_STATES_H_
