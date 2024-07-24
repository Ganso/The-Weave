#include <genesis.h>
#include "globals.h"

// Show or hide notes
void show_note(u8 nnote, bool visible)
{
    Sprite *pentsprite;
    Sprite *rodsprite;
    u8 *notesong;

    switch (nnote) 
    {
    case NOTE_MI:
        pentsprite=spr_int_pentagram_1;
        rodsprite=spr_int_rod_1;
        notesong=(u8*)snd_note_mi;
        break;
    case NOTE_FA:
        pentsprite=spr_int_pentagram_2;
        rodsprite=spr_int_rod_2;
        notesong=(u8*)snd_note_fa;
        break;
    case NOTE_SOL:
        pentsprite=spr_int_pentagram_3;
        rodsprite=spr_int_rod_3;
        notesong=(u8*)snd_note_sol;
        break;
    case NOTE_LA:
        pentsprite=spr_int_pentagram_4;
        rodsprite=spr_int_rod_4;
        notesong=(u8*)snd_note_la;
        break;
    case NOTE_SI:
        pentsprite=spr_int_pentagram_5;
        rodsprite=spr_int_rod_5;
        notesong=(u8*)snd_note_si;
        break;
    default:
        pentsprite=spr_int_pentagram_6;
        rodsprite=spr_int_rod_6;
        notesong=(u8*)snd_note_do;
        break;
    }

    if (visible == true) {
        SPR_setVisibility(pentsprite, VISIBLE);
        SPR_setVisibility(rodsprite, VISIBLE);
        XGM_setLoopNumber(0);
        XGM_startPlay(notesong);
    }
    else {
        SPR_setVisibility(pentsprite, HIDDEN);
        SPR_setVisibility(rodsprite, HIDDEN);
    }
}

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
            show_note(note_playing, false); // Hide the npte
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
            time_since_last_note=0;
            num_played_notes=0;
        }
    }
}
// Check the finished pattern
void check_pattern(void)
{
    num_played_notes=0;
    time_since_last_note=0;
}
