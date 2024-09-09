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

#define MAX_NOTE_PLAYING_TIME  500  // Note playing time in milliseconds (0.5 seconds)
#define MAX_PATTERN_WAIT_TIME 2000   // Time to wait for a next note before cancelling the pattern in milliseconds (2 seconds)

bool patterns_enabled; // Can the character play a pattern right now?

u8 note_playing; // The note the player is playing
u16 note_playing_time; // How long has the note been played (in ticks)
u16 time_since_last_note; // How long are we waiting for the pattern to finish
u16 pattern_effect_in_progress; // Is a pattern effect currently in progress?
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

Sprite *spr_pattern_list_note[4]; // The 4 notes in right of the pattern list


void play_note(u8 nnote); // Play a note
void check_note(void); // Check if a note is being played
void check_pattern(void); // Check the finished pattern
void play_pattern_sound(u16 npattern); // Play the sound of a pattern spell
void init_patterns(void); // initialize patters
void check_pattern_effect(void); // Check if a pattern has a current effect

#endif