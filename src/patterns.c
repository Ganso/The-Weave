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
        if (note_playing_time==MAX_NOTE_PLAYING_TIME) { // Finished
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
        if (time_since_last_note==MAX_PATTERN_WAIT_TIME) { // Done waiting. The pattern is cancelled
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
    u8 matches,matched_pattern;

    matched_pattern=254;
    for (npattern=0;npattern<MAX_PATTERNS;npattern++) {
        matches=0;
        for (nnote=0;nnote<4;nnote++) {
            if (played_notes[nnote]==obj_pattern[npattern].notes[nnote]) {
                matches++;
            }
        }
        if (matches==4 && obj_pattern[npattern].active==true) matched_pattern=npattern; // We have a match!
    }
    hide_rod_icons(); // Hide the rod pattern icons

    // Pattern effects
    if (matched_pattern!=254) { // We have a match!
        anim_character(active_character,ANIM_MAGIC); // Magic animation
        show_pattern_icon(matched_pattern, 96, true, true); // Show appropiate icon
        SPR_update();
        if (matched_pattern==PTRN_ELECTIC) { // THUNDER !!!
            u8 i;
            play_pattern_sound(PTRN_ELECTIC);
            for (i=0;i<100;i++) {
                VDP_setHilightShadow(true);
                SYS_doVBlankProcess();
                VDP_setHilightShadow(false);
                SYS_doVBlankProcess();
            }
        }
        if (matched_pattern==PTRN_HIDE) { // HIDE!!
            u8 i;
            play_pattern_sound(PTRN_HIDE);
            for (i=0;i<100;i++) {
                show_character(active_character, false);
                SYS_doVBlankProcess();
                show_character(active_character, true);
                SYS_doVBlankProcess();
            }
        }
        show_pattern_icon(matched_pattern, 96, false, false); // Hide the icon
        anim_character(active_character,ANIM_IDLE); // Stop magic animation
        SPR_update();
    }

    num_played_notes=0;
    time_since_last_note=0;
    next_frame();
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
        XGM2_playPCM(snd_pattern_hide,sizeof(snd_pattern_hide),SOUND_PCM_CH_AUTO);
        break;
    default: // Pattern: Electric
        XGM2_playPCM(snd_pattern_thunder,sizeof(snd_pattern_thunder),SOUND_PCM_CH_AUTO);
        break;
    }
}  

// initialize patters
void init_patterns(void)
{
    obj_pattern[PTRN_ELECTIC]=(Pattern) {true, {1,2,3,4}, NULL};
    obj_pattern[PTRN_HIDE]=(Pattern) {true, {2,5,3,6}, NULL};
    obj_pattern[PTRN_OPEN]=(Pattern) {true, {2,3,3,2}, NULL};

}