#ifndef _CHR_PATTERNS_H_
#define _CHR_PATTERNS_H_

#include "globals.h"

/**
 * @brief Pattern definition structure
 */
typedef struct {
    bool active;           // Whether pattern is unlocked
    u8 notes[4];          // Required note sequence
    Sprite *sd;           // Pattern sprite
} Pattern;

// Pattern state variables
extern bool player_patterns_enabled;              // Whether pattern system is enabled
extern u8 note_playing;                          // Current playing note (NOTE_NONE if none)
extern u16 note_playing_time;                    // Duration of current note
extern u16 time_since_last_note;                 // Time since last note in sequence
extern u16 player_pattern_effect_in_progress;    // Active pattern effect (PTRN_NONE if none)
extern bool player_pattern_effect_reversed;       // Whether pattern is reversed
extern u16 player_pattern_effect_time;           // Duration of current effect
extern u8 played_notes[4];                       // Sequence of played notes
extern u8 num_played_notes;                      // Number of notes played
extern Pattern obj_pattern[MAX_PATTERNS];         // Available patterns

/**
 * @brief Initialize pattern system
 */
void init_patterns(void);

/**
 * @brief Unlock new pattern with visual/audio feedback
 * @param npattern Pattern ID to unlock
 */
void activate_spell(u16 npattern);

/**
 * @brief Handle new note input
 * @param nnote Note ID to play
 */
void play_note(u8 nnote);

/**
 * @brief Process character states for pattern system
 */
void check_active_character_state(void);

/**
 * @brief Play pattern sound effect
 * @param npattern Pattern ID
 */
void play_pattern_sound(u16 npattern);

/**
 * @brief Check if notes match a pattern
 * @param notes Note sequence to check
 * @param is_reverse Set to true if pattern matched in reverse
 * @return Matched pattern ID or PTRN_NONE
 */
u8 validate_pattern_sequence(u8 *notes, bool *is_reverse);

// Pattern validation functions
bool can_use_electric_pattern(void);
bool can_use_hide_pattern(void);
bool can_use_sleep_pattern(void);
bool can_use_open_pattern(void);

// Pattern effect functions
void launch_electric_pattern(void);
void do_electric_pattern_effect(void);
void finish_electric_pattern_effect(void);

void launch_hide_pattern(void);
void do_hide_pattern_effect(void);
void finish_hide_pattern_effect(void);

void launch_sleep_pattern(void);
void do_sleep_pattern_effect(void);
void finish_sleep_pattern_effect(void);

void launch_open_pattern(void);
void do_open_pattern_effect(void);
void finish_open_pattern_effect(void);

// Pattern state management
void reset_player_pattern_state(void);
void handle_pattern_timeout(void);
void update_pattern_state(void);

#endif // _CHR_PATTERNS_H_
