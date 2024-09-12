#include <genesis.h>
#include "globals.h"

// Play a note
void play_note(u8 nnote)
{
    if (note_playing==NOTE_NONE) {
        if (note_playing_time==0) { // Note starting
            show_note(nnote, true);
            note_playing=nnote;
            note_playing_time++;
            played_notes[num_played_notes]=nnote;
            num_played_notes++;
        }
    }
}

// Check if a note is being played
void check_note(void)
{
    if (note_playing_time!=0) { // A note is being played
        if (note_playing_time==calc_ticks(MAX_NOTE_PLAYING_TIME)) { // Finished
            show_note(note_playing, false); // Hide the note
            note_playing=NOTE_NONE;
            time_since_last_note=1; // A pattern is possible. Start counting ticks to cancel it
            note_playing_time=0;
            if (num_played_notes==4) {
                check_pattern(); // Pattern finished
            }
        }
        else note_playing_time++; // Keep playing
    }
    else if (time_since_last_note!=0) {
        time_since_last_note++;
        if (time_since_last_note==calc_ticks(MAX_PATTERN_WAIT_TIME)) { // Done waiting. The pattern is cancelled
            hide_rod_icons();
            time_since_last_note=0;
            num_played_notes=0;
        }
    }
}

// Check the finished pattern
void check_pattern(void)
{
    u8 npattern,nnote; // Loop indexes
    u8 matches,reverse_matches,matched_pattern;
    bool is_reverse_match;
    u8 i;

    matched_pattern=254;
    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        matches=0;
        for (nnote=0;nnote<4;nnote++) {
            if (played_notes[nnote]==obj_pattern[npattern].notes[nnote]) {
                matches++;
            }
            if (played_notes[nnote]==obj_pattern[npattern].notes[3-nnote]) {
                reverse_matches++;
            }
        }
        if (matches==4 && obj_pattern[npattern].active==true) {
            matched_pattern=npattern; // We have a match!
            is_reverse_match=false;
        }
        else if (reverse_matches==4 && obj_pattern[npattern].active==true) {
            matched_pattern=npattern; // We have a reverse match!
            is_reverse_match=true;
        }
    }
    hide_rod_icons(); // Hide the rod pattern icons

    // Pattern effects
    if (matched_pattern!=254) { // We have a match!
        pattern_effect_reversed=false;
        if (matched_pattern==PTRN_ELECTRIC && is_reverse_match==false) { // THUNDER !!!
            anim_character(active_character,ANIM_MAGIC); // Magic animation
            show_pattern_icon(matched_pattern, true, true); // Show appropiate icon
            SPR_update();
            play_pattern_sound(PTRN_ELECTRIC);
            for (i=0;i<100;i++) {
                VDP_setHilightShadow(true);
                SYS_doVBlankProcess();
                VDP_setHilightShadow(false);
                SYS_doVBlankProcess();
            }
            show_pattern_icon(matched_pattern, false, false); // Hide the icon
            anim_character(active_character,ANIM_IDLE); // Stop magic animation
            SPR_update();
        }
        else if (matched_pattern==PTRN_HIDE && is_reverse_match==false) { // HIDE!!
            show_pattern_icon(matched_pattern, true, true); // Show appropiate icon
            play_pattern_sound(PTRN_HIDE);
            pattern_effect_in_progress=PTRN_HIDE;
            pattern_effect_time=1;
        }
        else if (matched_pattern==PTRN_ELECTRIC && is_reverse_match==true) { // Reverse THUNDER !!!
            if (is_combat_active==true && enemy_attacking!=ENEMY_NONE && attack_effect_in_progress==true && enemy_attack_pattern==PTRN_EN_ELECTIC) {
                pattern_effect_in_progress=PTRN_ELECTRIC;
                pattern_effect_reversed=true;
            }
        }
        else { // We have a match, but pattern is not usable right now
            show_pattern_icon(matched_pattern, true, true); // Show appropiate icon
            play_pattern_sound(matched_pattern); // Play pattern sound
            show_or_hide_interface(false);
            talk_dialog(&dialogs[SYSTEM_DIALOG][0]); // "I can't do it now"
            show_or_hide_interface(true);
            show_pattern_icon(matched_pattern, false, false); // Show appropiate icon
        }
    }
    else {
        play_pattern_sound(PTRN_NONE); // Failed pattern
    }
    num_played_notes=0;
    time_since_last_note=0;
    next_frame(false);
}

// Play the sound of a pattern spell
void play_pattern_sound(u16 npattern)
{
    switch (npattern)
    {
    case PTRN_HIDE:
        XGM2_playPCM(snd_pattern_hide,sizeof(snd_pattern_hide),SOUND_PCM_CH_AUTO);
        break;
    case PTRN_OPEN:
        XGM2_playPCM(snd_pattern_open,sizeof(snd_pattern_open),SOUND_PCM_CH_AUTO);
        break;
    case PTRN_ELECTRIC: // Pattern: Electric
        XGM2_playPCM(snd_pattern_thunder,sizeof(snd_pattern_thunder),SOUND_PCM_CH_AUTO);
        break;
    default: // Invalid pattern
        XGM2_playPCM(snd_pattern_invalid,sizeof(snd_pattern_invalid),SOUND_PCM_CH_AUTO);
        break;        
    }
}  

// initialize patters
void init_patterns(void)
{
    obj_pattern[PTRN_ELECTRIC]=(Pattern) {true, {1,2,3,4}, NULL};
    obj_pattern[PTRN_HIDE]=(Pattern) {true, {2,5,3,6}, NULL};
    obj_pattern[PTRN_OPEN]=(Pattern) {true, {2,3,3,2}, NULL};

}

// Check if a pattern has a current effect
void check_pattern_effect(void)
{
    u16 max_effect_time;
    max_effect_time=400;

    switch (pattern_effect_in_progress)
    {
    case PTRN_HIDE:
        if (pattern_effect_time!=max_effect_time) {
            if (pattern_effect_time%2==0) show_character(active_character,true);
            else show_character(active_character,false);
            pattern_effect_time++;
        }
        else {
            show_pattern_icon(PTRN_HIDE, false, false); // Show appropiate icon
            show_character(active_character,true);
            pattern_effect_in_progress=PTRN_EN_NONE;
            pattern_effect_time=0;
        }
        break;
    
    default:
        break;
    }
}