#include "globals.h"
#include "character_patterns.h"

// Pattern state variables
bool player_patterns_enabled = false;
u8 note_playing = NOTE_NONE;
u16 note_playing_time = 0;
u16 time_since_last_note = 0;
u16 player_pattern_effect_in_progress = PTRN_NONE;
bool player_pattern_effect_reversed = false;
u16 player_pattern_effect_time = 0;
u8 played_notes[4] = {0};
u8 num_played_notes = 0;
Pattern obj_pattern[MAX_PATTERNS];

// Debug output macro
#define PATTERN_DEBUG(fmt, ...) \
    if (DEBUG_STATE_MACHINES) { \
        kprintf("[PLAYER_PATTERN] " fmt "\n", ##__VA_ARGS__); \
    }

void init_patterns(void)
{
    u16 i;
    
    // Initialize pattern states
    for (i = 0; i < MAX_PATTERNS; i++) {
        obj_pattern[i].active = false;
        obj_pattern[i].sd = NULL;
    }
    
    // Reset pattern state
    reset_player_pattern_state();
    
    PATTERN_DEBUG("Pattern system initialized");
}

void activate_spell(u16 npattern)
{
    if (npattern < MAX_PATTERNS) {
        PATTERN_DEBUG("Activating pattern %d", npattern);
        obj_pattern[npattern].active = true;
        play_pattern_sound(npattern);
    }
}

void play_note(u8 nnote)
{
    if (!player_patterns_enabled || note_playing != NOTE_NONE) {
        return;
    }
    
    PATTERN_DEBUG("Playing note %d", nnote);
    
    // Record note in sequence
    if (num_played_notes < 4) {
        played_notes[num_played_notes++] = nnote;
    }
    
    // Start note playback
    note_playing = nnote;
    note_playing_time = 0;
    time_since_last_note = 0;
}

void check_active_character_state(void)
{
    // Update note timers
    if (note_playing != NOTE_NONE) {
        note_playing_time++;
        if (note_playing_time >= MAX_NOTE_PLAYING_TIME) {
            PATTERN_DEBUG("Note %d finished", note_playing);
            note_playing = NOTE_NONE;
        }
    } else if (num_played_notes > 0) {
        time_since_last_note++;
        if (time_since_last_note >= MAX_PATTERN_WAIT_TIME) {
            PATTERN_DEBUG("Pattern timeout");
            handle_pattern_timeout();
        }
    }
    
    // Update pattern effects
    if (player_pattern_effect_in_progress != PTRN_NONE) {
        player_pattern_effect_time++;
        update_pattern_state();
    }
}

void play_pattern_sound(u16 npattern)
{
    switch (npattern) {
        case PTRN_ELECTRIC:
            play_sample(snd_pattern_thunder, sizeof(snd_pattern_thunder));
            break;
        case PTRN_HIDE:
            play_sample(snd_pattern_hide, sizeof(snd_pattern_hide));
            break;
        case PTRN_OPEN:
            play_sample(snd_pattern_open, sizeof(snd_pattern_open));
            break;
        case PTRN_SLEEP:
            play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
            break;
    }
}

u8 validate_pattern_sequence(u8 *notes, bool *is_reverse)
{
    u16 i, j;
    bool match, reverse_match;
    
    // Check each pattern
    for (i = 0; i < MAX_PATTERNS; i++) {
        if (!obj_pattern[i].active) {
            continue;
        }
        
        // Check forward match
        match = true;
        for (j = 0; j < num_played_notes; j++) {
            if (notes[j] != obj_pattern[i].notes[j]) {
                match = false;
                break;
            }
        }
        
        // Check reverse match
        reverse_match = true;
        for (j = 0; j < num_played_notes; j++) {
            if (notes[j] != obj_pattern[i].notes[num_played_notes - 1 - j]) {
                reverse_match = false;
                break;
            }
        }
        
        // Return pattern if matched
        if (match || reverse_match) {
            *is_reverse = reverse_match;
            PATTERN_DEBUG("Pattern %d matched (reverse: %d)", i, reverse_match);
            return i;
        }
    }
    
    return PTRN_NONE;
}

void reset_player_pattern_state(void)
{
    PATTERN_DEBUG("Resetting pattern state");
    
    note_playing = NOTE_NONE;
    note_playing_time = 0;
    time_since_last_note = 0;
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_reversed = false;
    player_pattern_effect_time = 0;
    num_played_notes = 0;
}

void handle_pattern_timeout(void)
{
    bool is_reverse;
    u8 matched_pattern;
    
    // Check for pattern match
    matched_pattern = validate_pattern_sequence(played_notes, &is_reverse);
    
    if (matched_pattern != PTRN_NONE) {
        // Pattern matched - start effect
        player_pattern_effect_in_progress = matched_pattern;
        player_pattern_effect_reversed = is_reverse;
        player_pattern_effect_time = 0;
        
        // Launch pattern-specific effect
        switch (matched_pattern) {
            case PTRN_ELECTRIC:
                launch_electric_pattern();
                break;
            case PTRN_HIDE:
                launch_hide_pattern();
                break;
            case PTRN_OPEN:
                launch_open_pattern();
                break;
            case PTRN_SLEEP:
                launch_sleep_pattern();
                break;
        }
    }
    
    // Reset pattern state
    reset_player_pattern_state();
}

void update_pattern_state(void)
{
    u16 max_effect_time;
    
    // Get max effect time for current pattern
    switch (player_pattern_effect_in_progress) {
        case PTRN_ELECTRIC:
            max_effect_time = MAX_EFFECT_TIME_ELECTRIC;
            break;
        default:
            max_effect_time = 100;
    }
    
    // Update pattern effect
    if (player_pattern_effect_time < max_effect_time) {
        switch (player_pattern_effect_in_progress) {
            case PTRN_ELECTRIC:
                do_electric_pattern_effect();
                break;
            case PTRN_HIDE:
                do_hide_pattern_effect();
                break;
            case PTRN_OPEN:
                do_open_pattern_effect();
                break;
            case PTRN_SLEEP:
                do_sleep_pattern_effect();
                break;
        }
    } else {
        // Effect finished
        switch (player_pattern_effect_in_progress) {
            case PTRN_ELECTRIC:
                finish_electric_pattern_effect();
                break;
            case PTRN_HIDE:
                finish_hide_pattern_effect();
                break;
            case PTRN_OPEN:
                finish_open_pattern_effect();
                break;
            case PTRN_SLEEP:
                finish_sleep_pattern_effect();
                break;
        }
        
        player_pattern_effect_in_progress = PTRN_NONE;
    }
}

// Pattern validation functions
bool can_use_electric_pattern(void) { return obj_pattern[PTRN_ELECTRIC].active; }
bool can_use_hide_pattern(void) { return obj_pattern[PTRN_HIDE].active; }
bool can_use_sleep_pattern(void) { return obj_pattern[PTRN_SLEEP].active; }
bool can_use_open_pattern(void) { return obj_pattern[PTRN_OPEN].active; }

// Pattern effect functions
void launch_electric_pattern(void) { /* Implemented in combat.c */ }
void do_electric_pattern_effect(void) { /* Implemented in combat.c */ }
void finish_electric_pattern_effect(void) { /* Implemented in combat.c */ }

void launch_hide_pattern(void) { /* Implemented in combat.c */ }
void do_hide_pattern_effect(void) { /* Implemented in combat.c */ }
void finish_hide_pattern_effect(void) { /* Implemented in combat.c */ }

void launch_sleep_pattern(void) { /* Implemented in combat.c */ }
void do_sleep_pattern_effect(void) { /* Implemented in combat.c */ }
void finish_sleep_pattern_effect(void) { /* Implemented in combat.c */ }

void launch_open_pattern(void) { /* Implemented in combat.c */ }
void do_open_pattern_effect(void) { /* Implemented in combat.c */ }
void finish_open_pattern_effect(void) { /* Implemented in combat.c */ }
