#ifndef _PATTERNS_H_
#define _PATTERNS_H_

// Notes
u8 note_playing; // The note the player is playing
u16 note_playing_time; // How long has the note been played (in ticks)
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6

void show_note(u8 nnote, bool visible); // Show or hide notespentsprite
void play_note(u8 nnote); // Play a note

#endif