#ifndef _PATTERNS_H_
#define _PATTERNS_H_

// Notes
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6

#define MAX_NOTE_PLAYING_TIME  60   // Note playing time (60 ticks, 1 second)
#define MAX_PATTERN_WAIT_TIME 120   // Time to wait for a next note before cancelling the pattern (120 ticks, 2 seconds)

u8 note_playing; // The note the player is playing
u16 note_playing_time; // How long has the note been played (in ticks)
u16 time_since_last_note; // How long are we waiting for the pattern to finish

// Patterns
#define MAX_PATTERNS 2
#define PTRN_NONE         254
#define PTRN_ELECTIC      0   // Electricity spell
#define PTRN_HIDE         1   // Hide spell

u8 played_notes[4]; // Notes played in the current pattern
u8 num_played_notes; // Number of notes of the current pattern

typedef struct
{
    bool active;
    u8 notes[4];
    Sprite *sd;
} Pattern;
Pattern obj_pattern[MAX_PATTERNS]; // Patterns object

Sprite *spr_pattern_list_note[4]; // The 4 notes in right of the pattern list


void show_note(u8 nnote, bool visible); // Show or hide notespentsprite
void play_note(u8 nnote); // Play a note
void check_note(void); // Check if a note is being played
void check_pattern(void); // Check the finished pattern
void hide_rod_icons(void); // Hide icons in the rod
void play_pattern_sound(u16 npattern); // Play the sound of a pattern spell
void show_pattern_icon(u16 npattern, u16 x, bool show, bool priority); // Show or hide the icon of a pattern spell

#endif