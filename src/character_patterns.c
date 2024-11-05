#include <genesis.h>
#include "globals.h"

// Play a note
void play_note(u8 nnote)
{
    if (note_playing == NOTE_NONE) {
        if (note_playing_time == 0) { // Note starting
            show_note(nnote, true);
            note_playing = nnote;
            note_playing_time++;
            played_notes[num_played_notes] = nnote;
            num_played_notes++;
            
            // Change character state to playing note
            obj_character[active_character].state = STATE_PLAYING_NOTE;

            // Play animaton
            update_character_animation();
        }
    }
}

// Main state machine for pattern system
void check_character_state(void)
{
    u8 npattern, nnote; // Loop indexes
    u8 matches, reverse_matches, matched_pattern;
    bool is_reverse_match;
    u8 i;
    u16 max_effect_time = 400;

    // Main state machine
    switch (obj_character[active_character].state)
    {
        case STATE_PLAYING_NOTE:
            // Handle note playing state
            if (note_playing_time != 0) {
                if (note_playing_time == calc_ticks(MAX_NOTE_PLAYING_TIME)) {
                    // Note finished playing
                    show_note(note_playing, false);
                    note_playing = NOTE_NONE;
                    time_since_last_note = 1;
                    note_playing_time = 0;
                    
                    if (num_played_notes == 4) {
                        // All notes collected, move to pattern finished
                        obj_character[active_character].state = STATE_PATTERN_FINISHED;
                        // Directly move to pattern check
                        obj_character[active_character].state = STATE_PATTERN_CHECK;
                    } else {
                        // Still collecting notes, return to idle
                        obj_character[active_character].state = STATE_IDLE;
                    }
                    update_character_animation();
                }
                else note_playing_time++;
            }
            break;

        case STATE_IDLE:
            // Check for pattern timeout
            if (time_since_last_note != 0) {
                time_since_last_note++;
                if (time_since_last_note == calc_ticks(MAX_PATTERN_WAIT_TIME)) {
                    hide_rod_icons();
                    time_since_last_note = 0;
                    num_played_notes = 0;
                }
            }
            break;

        case STATE_PATTERN_CHECK:
            // Check for pattern match
            matched_pattern = 254;
            for (npattern = 0; npattern < MAX_PATTERNS; npattern++) {
                matches = 0;
                reverse_matches = 0;
                for (nnote = 0; nnote < 4; nnote++) {
                    if (played_notes[nnote] == obj_pattern[npattern].notes[nnote]) {
                        matches++;
                    }
                    if (played_notes[nnote] == obj_pattern[npattern].notes[3-nnote]) {
                        reverse_matches++;
                    }
                }
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
                if (matched_pattern == PTRN_ELECTRIC && !is_reverse_match) {
                    if (is_combat_active && enemy_attacking != ENEMY_NONE && 
                        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                        // Wrong pattern during thunder attack
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
                        // Thunder effect
                        obj_character[active_character].state = STATE_PATTERN_EFFECT;
                        anim_character(active_character, ANIM_MAGIC);
                        show_pattern_icon(matched_pattern, true, true);
                        SPR_update();
                        play_pattern_sound(PTRN_ELECTRIC);
                        for (i = 0; i < 100; i++) {
                            VDP_setHilightShadow(true);
                            SYS_doVBlankProcess();
                            VDP_setHilightShadow(false);
                            SYS_doVBlankProcess();
                        }
                        show_pattern_icon(matched_pattern, false, false);
                        SPR_update();
                        
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
                else if (matched_pattern == PTRN_HIDE && !is_reverse_match) {
                    // Hide effect
                    obj_character[active_character].state = STATE_PATTERN_EFFECT;
                    show_pattern_icon(matched_pattern, true, true);
                    play_pattern_sound(PTRN_HIDE);
                    movement_active = true; // Permitir movimiento durante ocultación
                    pattern_effect_in_progress = PTRN_HIDE;
                    pattern_effect_time = 1;
                }
                else if (matched_pattern == PTRN_ELECTRIC && is_reverse_match && 
                         pattern_effect_in_progress == PTRN_NONE && 
                         is_combat_active && enemy_attacking != ENEMY_NONE && 
                         enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
                    // Reverse thunder during enemy thunder attack
                    obj_character[active_character].state = STATE_PATTERN_EFFECT;
                    pattern_effect_in_progress = PTRN_ELECTRIC;
                    pattern_effect_reversed = true;
                }
                else {
                    // Pattern matched but not usable now
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
                // No pattern match
                play_pattern_sound(PTRN_NONE);
                obj_character[active_character].state = STATE_IDLE;
            }
            
            num_played_notes = 0;
            time_since_last_note = 0;
            next_frame(false);
            break;

        case STATE_PATTERN_EFFECT:
            // Handle ongoing pattern effects
            if (pattern_effect_in_progress == PTRN_HIDE) {
                movement_active = true; // Mantener el movimiento activo durante el efecto
                if (pattern_effect_time != max_effect_time) {
                    if (pattern_effect_time % 2 == 0) {
                        show_character(active_character, true);
                    } else {
                        show_character(active_character, false);
                    }
                    pattern_effect_time++;
                }
                else {
                    show_pattern_icon(PTRN_HIDE, false, false);
                    show_character(active_character, true);
                    pattern_effect_in_progress = PTRN_NONE;
                    pattern_effect_time = 0;
                    obj_character[active_character].state = STATE_PATTERN_EFFECT_FINISH;
                }
            }
            break;

        case STATE_PATTERN_EFFECT_FINISH:
            // Reset to idle after effect finishes
            obj_character[active_character].state = STATE_IDLE;
            break;

        default:
            break;
    }
}

// Play the sound of a pattern spell
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
    case PTRN_ELECTRIC: // Pattern: Electric
        XGM2_playPCM(snd_pattern_thunder, sizeof(snd_pattern_thunder), SOUND_PCM_CH_AUTO);
        break;
    default: // Invalid pattern
        XGM2_playPCM(snd_pattern_invalid, sizeof(snd_pattern_invalid), SOUND_PCM_CH_AUTO);
        break;        
    }
}  

// initialize patters
void init_patterns(void)
{
    obj_pattern[PTRN_ELECTRIC] = (Pattern) {true, {1,2,3,4}, NULL};
    obj_pattern[PTRN_HIDE] = (Pattern) {true, {2,5,3,6}, NULL};
    obj_pattern[PTRN_OPEN] = (Pattern) {true, {2,3,3,2}, NULL};
}
