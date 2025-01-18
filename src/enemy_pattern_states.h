#ifndef _ENEMY_PATTERN_STATES_H_
#define _ENEMY_PATTERN_STATES_H_

#include "globals.h"
#include "enemies_patterns.h"

/**
 * @brief Enemy pattern state machine data
 */
typedef struct {
    StateMachine base;           // Base state machine (must be first)
    
    // Pattern data
    u16 enemy_id;               // Enemy casting the pattern
    u16 pattern_id;             // Pattern being cast
    u16 current_note;           // Current note in sequence
    u16 note_timer;             // Timer for current note
    bool was_interrupted;       // Whether pattern was interrupted
    
    // Effect data
    u16 effect_timer;          // Timer for pattern effect
    bool effect_active;        // Whether effect is currently active
} EnemyPatternStateMachine;

// Enemy pattern states
extern const State STATE_ENEMY_IDLE;           // No pattern active
extern const State STATE_ENEMY_NOTE_SEQUENCE;  // Playing pattern notes
extern const State STATE_ENEMY_EFFECT_START;   // Starting pattern effect
extern const State STATE_ENEMY_EFFECT_ACTIVE;  // Pattern effect in progress
extern const State STATE_ENEMY_EFFECT_END;     // Pattern effect finishing

// Current state machine instance (used by pattern effect functions)
extern EnemyPatternStateMachine* current_epsm;

/**
 * @brief Initialize an enemy pattern state machine
 * @param epsm State machine to initialize
 * @param debug Enable debug output
 */
void enemy_pattern_sm_init(EnemyPatternStateMachine* epsm, bool debug);

/**
 * @brief Update enemy pattern state machine
 * @param epsm State machine to update
 */
void enemy_pattern_sm_update(EnemyPatternStateMachine* epsm);

/**
 * @brief Start enemy pattern casting
 * @param epsm State machine
 * @param enemy_id ID of casting enemy
 * @param pattern_id ID of pattern to cast
 * @return true if pattern started, false if another cast in progress
 */
bool enemy_pattern_sm_start(EnemyPatternStateMachine* epsm, u16 enemy_id, u16 pattern_id);

/**
 * @brief Interrupt current pattern casting
 * @param epsm State machine
 * @param source_id ID of who caused interruption
 */
void enemy_pattern_sm_interrupt(EnemyPatternStateMachine* epsm, u16 source_id);

/**
 * @brief Check if enemy pattern state machine is active
 * @param epsm State machine to check
 * @return true if pattern casting is in progress
 */
bool enemy_pattern_sm_is_active(const EnemyPatternStateMachine* epsm);

#endif // _ENEMY_PATTERN_STATES_H_
