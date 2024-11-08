#include <genesis.h>
#include "globals.h"

/**
 * Character Pattern System
 * 
 * This module handles the musical pattern system used by characters, including:
 * - Note playing and pattern input collection
 * - Pattern matching and validation
 * - Pattern effect execution (thunder, hide, etc.)
 * - State management for the pattern system
 */

/**
 * Process a new note being played
 * Initializes note playing state and updates character animation
 */
void play_note(u8 nnote)
{
    if (note_playing == NOTE_NONE) {
        if (note_playing_time == 0) {
            // Start playing the note
            show_note(nnote, true);
            note_playing = nnote;
            note_playing_time++;
            played_notes[num_played_notes] = nnote;
            num_played_notes++;
            
            // Update character state
            obj_character[active_character].state = STATE_PLAYING_NOTE;
            update_character_animation();
        }
    }
}

/**
 * Main state machine for pattern system
 * Handles all states of pattern input and execution:
 * - STATE_PLAYING_NOTE: Processing individual note input
 * - STATE_IDLE: Waiting for input, handling timeouts
 * - STATE_PATTERN_CHECK: Validating completed patterns
 * - STATE_PATTERN_EFFECT: Executing pattern effects
 * - STATE_PATTERN_EFFECT_FINISH: Cleanup after effects
 */
void check_active_character_state(void)
{
    u8 npattern, nnote;
    u8 matches, reverse_matches, matched_pattern;
    bool is_reverse_match;
    u8 i;
    u16 max_effect_time = 400;

    switch (obj_character[active_character].state)
    {
        case STATE_PLAYING_NOTE:
            // Handle note duration and completion
            if (note_playing_time != 0) {
                if (note_playing_time == calc_ticks(MAX_NOTE_PLAYING_TIME)) {
                    // Note finished playing
                    show_note(note_playing, false);
                    note_playing = NOTE_NONE;
                    time_since_last_note = 1;
                    note_playing_time = 0;
                    
                    // Check if pattern is complete (4 notes)
                    if (num_played_notes == 4) {
                        obj_character[active_character].state = STATE_PATTERN_FINISHED;
                        obj_character[active_character].state = STATE_PATTERN_CHECK;
                    } else {
                        obj_character[active_character].state = STATE_IDLE;
                    }
                    update_character_animation();
                }
                else note_playing_time++;
            }
            break;

        case STATE_IDLE:
            // Check for pattern input timeout
            if (time_since_last_note != 0) {
                time_since_last_note++;
                if (time_since_last_note == calc_ticks(MAX_PATTERN_WAIT_TIME)) {
                    // Pattern timed out, reset state
                    hide_rod_icons();
                    time_since_last_note = 0;
                    num_played_notes = 0;
                }
            }
            break;

        case STATE_PATTERN_CHECK:
            // Check if input matches any known pattern
            matched_pattern = 254;  // No match initially
            for (npattern = 0; npattern < MAX_PATTERNS; npattern++) {
                matches = 0;
                reverse_matches = 0;
                // Check both forward and reverse pattern matches
                for (nnote = 0; nnote < 4; nnote++) {
                    if (played_notes[nnote] == obj_pattern[npattern].notes[nnote]) {
                        matches++;
                    }
                    if (played_notes[nnote] == obj_pattern[npattern].notes[3-nnote]) {
                        reverse_matches++;
                    }
                }
                // Pattern found if all notes match and pattern is active
                if (matches == 4 && obj_pattern[npattern].active == true) {
                    matched_pattern = npattern;
                    is_reverse_match = false;
                }
                else if (reverse_matches == 4 && obj_pattern[npattern].active == true) {
                    matched_pattern = npattern;
                    is_reverse_match = true;
                }
            }
            hide_rod_icons();

            if (matched_pattern != 254) {
                pattern_effect_reversed = false;
                // Handle thunder pattern
                if (matched_pattern == PTRN_ELECTRIC && !is_reverse_match) {
                    if (is_combat_active && enemy_attacking != ENEMY_NONE && 
                        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                        // Can't use thunder during enemy thunder attack
                        show_or_hide_interface(false);
                        show_or_hide_enemy_combat_interface(false);
                        talk_dialog(&dialogs[ACT1_DIALOG3][6]);
                        show_or_hide_enemy_combat_interface(true);
                        show_or_hide_interface(true);
                        obj_character[active_character].state = STATE_IDLE;
                    }
                    else if (pattern_effect_in_progress == PTRN_HIDE) {
                        // Can't use thunder while hidden
                        show_or_hide_interface(false);
                        show_or_hide_enemy_combat_interface(false);
                        talk_dialog(&dialogs[ACT1_DIALOG3][9]);
                        show_or_hide_enemy_combat_interface(true);
                        show_or_hide_interface(true);
                        obj_character[active_character].state = STATE_IDLE;
                    }
                    else {
                        // Execute thunder effect
                        obj_character[active_character].state = STATE_PATTERN_EFFECT;
                        anim_character(active_character, ANIM_MAGIC);
                        show_pattern_icon(matched_pattern, true, true);
                        SPR_update();
                        play_pattern_sound(PTRN_ELECTRIC);
                        
                        // Visual thunder effect
                        for (i = 0; i < 100; i++) {
                            VDP_setHilightShadow(true);
                            SYS_doVBlankProcess();
                            VDP_setHilightShadow(false);
                            SYS_doVBlankProcess();
                        }
                        show_pattern_icon(matched_pattern, false, false);
                        SPR_update();
                        
                        // Apply combat effects
                        if (is_combat_active) {
                            for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
                                if (obj_enemy[nenemy].obj_character.active && 
                                    obj_enemy[nenemy].class_id == ENEMY_CLS_3HEADMONKEY && 
                                    obj_enemy[nenemy].hitpoints > 0) {
                                    hit_enemy(nenemy);
                                    if (enemy_attack_pattern == PTRN_EN_BITE) {
                                        enemy_attack_pattern = PTRN_EN_NONE;
                                        finish_enemy_pattern_effect();
                                    }
                                }
                            }
                        }
                        anim_character(active_character, ANIM_IDLE);
                        obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
                    }
                }
                // Handle hide pattern
                else if (matched_pattern == PTRN_HIDE && !is_reverse_match) {
                    obj_character[active_character].state = STATE_PATTERN_EFFECT;
                    show_pattern_icon(matched_pattern, true, true);
                    play_pattern_sound(PTRN_HIDE);
                    movement_active = true;  // Allow movement while hidden
                    pattern_effect_in_progress = PTRN_HIDE;
                    pattern_effect_time = 1;
                }
                // Handle thunder counter (reverse thunder during enemy thunder)
                else if (matched_pattern == PTRN_ELECTRIC && is_reverse_match && 
                         pattern_effect_in_progress == PTRN_NONE && 
                         is_combat_active && enemy_attacking != ENEMY_NONE && 
                         enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                    obj_character[active_character].state = STATE_PATTERN_EFFECT;
                    pattern_effect_in_progress = PTRN_ELECTRIC;
                    pattern_effect_reversed = true;
                }
                else {
                    // Pattern matched but not usable in current context
                    show_pattern_icon(matched_pattern, true, true);
                    play_pattern_sound(PTRN_NONE);
                    show_or_hide_interface(false);
                    talk_dialog(&dialogs[SYSTEM_DIALOG][0]);
                    show_or_hide_interface(true);
                    show_pattern_icon(matched_pattern, false, false);
                    obj_character[active_character].state = STATE_IDLE;
                }
            }
            else {
                // No pattern matched
                play_pattern_sound(PTRN_NONE);
                obj_character[active_character].state = STATE_IDLE;
            }
            
            // Reset pattern input state
            num_played_notes = 0;
            time_since_last_note = 0;
            next_frame(false);
            break;

        case STATE_PATTERN_EFFECT:
            // Handle ongoing hide effect
            if (pattern_effect_in_progress == PTRN_HIDE) {
                movement_active = true;  // Keep movement enabled during effect
                if (pattern_effect_time != max_effect_time) {
                    // Create flickering effect
                    if (pattern_effect_time % 2 == 0) {
                        show_character(active_character, true);
                    } else {
                        show_character(active_character, false);
                    }
                    pattern_effect_time++;
                }
                else {
                    // Effect complete
                    show_pattern_icon(PTRN_HIDE, false, false);
                    show_character(active_character, true);
                    pattern_effect_in_progress = PTRN_NONE;
                    pattern_effect_time = 0;
                    obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
                }
            }
            break;

        case STATE_PATTERN_EFFECT_FINISH:
            // Return to idle after effect completes
            obj_character[active_character].state = STATE_IDLE;
            break;

        default:
            break;
    }
}

/**
 * Play the appropriate sound effect for a pattern spell
 * Each pattern has its own distinct sound
 */
void play_pattern_sound(u16 npattern)
{
    switch (npattern)
    {
    case PTRN_HIDE:
        XGM2_playPCM(snd_pattern_hide, sizeof(snd_pattern_hide), SOUND_PCM_CH_AUTO);
        break;
    case PTRN_OPEN:
        XGM2_playPCM(snd_pattern_open, sizeof(snd_pattern_open), SOUND_PCM_CH_AUTO);
        break;
    case PTRN_ELECTRIC:  // Thunder pattern
        XGM2_playPCM(snd_pattern_thunder, sizeof(snd_pattern_thunder), SOUND_PCM_CH_AUTO);
        break;
    default:  // Invalid/unmatched pattern
        XGM2_playPCM(snd_pattern_invalid, sizeof(snd_pattern_invalid), SOUND_PCM_CH_AUTO);
        break;        
    }
}  

/**
 * Initialize the available patterns
 * Sets up the note sequences for each pattern:
 * - PTRN_ELECTRIC: MI-FA-SOL-LA (Thunder spell)
 * - PTRN_HIDE: FA-SI-SOL-DO (Hide spell)
 * - PTRN_OPEN: FA-SOL-SOL-FA (Open spell)
 */
void init_patterns(void)
{
    obj_pattern[PTRN_ELECTRIC] = (Pattern) {true, {1,2,3,4}, NULL};
    obj_pattern[PTRN_HIDE] = (Pattern) {true, {2,5,3,6}, NULL};
    obj_pattern[PTRN_OPEN] = (Pattern) {true, {2,3,3,2}, NULL};
}
