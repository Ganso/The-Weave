#ifndef _ENM_PATTERNS_H_
#define _ENM_PATTERNS_H_

#include "globals.h"

/**
 * @brief Enemy pattern definition structure
 */
typedef struct {
    u8 numnotes;           // Number of notes in sequence
    u8 notes[4];          // Note sequence (1-6:MI-DO)
    u16 recharge_time;    // Ticks between pattern uses
} Pattern_Enemy;

// Pattern definitions
extern Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY];

// Currently attacking enemy tracking
extern u16 enemy_attacking;         // Which enemy is attacking
extern u16 enemy_attack_pattern;    // Which pattern is being used

// Note indicator states
extern bool enemy_note_active[6];   // Is note # (MI-DO) active

/**
 * @brief Initialize enemy pattern system
 * 
 * Sets up:
 * - Pattern definitions
 * - Pattern state machine
 * - Initial states
 */
void init_enemy_patterns(void);

/**
 * @brief Update enemy pattern system
 * 
 * - Updates pattern state machine
 * - Checks for new pattern opportunities
 * - Manages pattern cooldowns
 */
void check_enemy_state(void);

/**
 * @brief Show/hide and optionally play enemy note
 * @param nnote Note number (1-6:MI-DO)
 * @param visible Whether to show note
 * @param play Whether to play note sound
 */
void show_enemy_note(u8 nnote, bool visible, bool play);

/**
 * @brief Clean up pattern effect and reset states
 */
void finish_enemy_pattern_effect(void);

/**
 * @brief Clean up all active enemy notes
 */
void cleanup_enemy_notes(void);

// Pattern-specific effect functions
void launch_electric_enemy_pattern(void);
void do_electric_enemy_pattern_effect(void);
void finish_electric_enemy_pattern_effect(void);

void launch_bite_enemy_pattern(void);
void do_bite_enemy_pattern_effect(void);
void finish_bite_enemy_pattern_effect(void);

#endif // _ENM_PATTERNS_H_
