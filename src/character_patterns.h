#ifndef _CHR_PATTERNS_H_
#define _CHR_PATTERNS_H_

// Notes
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6

#define MAX_NOTE_PLAYING_TIME  500  // Note playing time in milliseconds
#define MAX_PATTERN_WAIT_TIME 2000   // Time to wait for a next note before cancelling the pattern in milliseconds

bool patterns_enabled; // Can the character play a pattern right now?

u8 note_playing; // The note the player is playing
u16 note_playing_time; // How long has the note been played (in ticks)
u16 time_since_last_note; // How long are we waiting for the pattern to finish
u16 pattern_effect_in_progress; // Is a pattern effect currently in progress? Which one?
bool pattern_effect_reversed; // Is the effect of a reverse pattern?
u16 pattern_effect_time; // How long is the effect been active?

// Patterns
#define MAX_PATTERNS 3
#define PTRN_NONE         254
#define PTRN_ELECTRIC      0   // Electricity spell
#define PTRN_HIDE          1   // Hide spell
#define PTRN_OPEN          2   // Open spell

u8 played_notes[4]; // Notes played in the current pattern
u8 num_played_notes; // Number of notes of the current pattern

typedef struct
{
    bool active;
    u8 notes[4];
    Sprite *sd;
} Pattern;

Pattern obj_pattern[MAX_PATTERNS]; // Patterns object

// Core pattern system functions
void play_note(u8 nnote); // Play a note
void check_active_character_state(void); // Main state machine for pattern system
void play_pattern_sound(u16 npattern); // Play the sound of a pattern spell
void init_patterns(void); // initialize patterns

// Pattern validation functions
bool validate_pattern_sequence(u8 *notes, u8 pattern_id, bool *is_reverse); // Check if notes match a pattern
bool can_use_electric_pattern(void); // Check if thunder pattern can be used
bool can_use_hide_pattern(void); // Check if hide pattern can be used

// Pattern-specific effect functions
void launch_electric_pattern(void); // Initialize thunder pattern effect
void do_electric_pattern_effect(void); // Process ongoing thunder pattern effect
void finish_electric_pattern_effect(void); // Complete thunder pattern effect

void launch_hide_pattern(void); // Initialize hide pattern effect
void do_hide_pattern_effect(void); // Process ongoing hide pattern effect
void finish_hide_pattern_effect(void); // Complete hide pattern effect

// Pattern state management
void reset_pattern_state(void); // Reset pattern input state
void handle_pattern_timeout(void); // Handle pattern input timeout
void update_pattern_state(void); // Update pattern state after note completion

#endif
