#ifndef PATTERNS_H
#define PATTERNS_H

#include "globals.h"

// Include all patern headers
#include "patterns/pattern_open.h"
#include "patterns/pattern_hide.h"
#include "patterns/pattern_sleep.h"
#include "patterns/pattern_thunder.h"
#include "patterns/pattern_en_thunder.h"
#include "patterns/pattern_en_bite.h"

/* Max */
#define MAX_PLAYER_PATTERNS   6   /* maximum slots for player spells     */
#define MAX_ENEMY_PATTERNS    8   /* maximum slots for enemy spells      */

/* Player patterns  */
#define PATTERN_THUNDER   0
#define PATTERN_HIDE       1
#define PATTERN_OPEN       2
#define PATTERN_SLEEP      3
#define PATTERN_PLAYER_COUNT 4
#define PATTERN_PLAYER_NONE   254

/* Enemy patterns */
#define PATTERN_EN_THUNDER   0
#define PATTERN_EN_BITE       1

#define PATTERN_ENEMY_COUNT   2
#define PATTERN_ENEMY_NONE    254

// Notes
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6

// Timing and delays
#define MAX_NOTE_PLAYING_TIME  500  // Note playing time in milliseconds
#define MAX_PATTERN_WAIT_TIME 2000   // Time to wait for a next note before cancelling the pattern in milliseconds
#define MIN_TIME_BETWEEN_NOTES SCREEN_FPS/4    // Minimum ticks between consecutive notes to prevent too rapid input (0.25 seconds)
#define MIN_TIME_BETWEEN_PATTERNS SCREEN_FPS/4 // Minimum ticks between consecutive patterns (0.25 seconds)
#define ENEMY_FRAMES_PER_NOTE SCREEN_FPS/4 // Enemy frames per note (0.5 seconds per note)

// Globals
extern bool player_has_rod;          /* can physically use patterns?      */
extern bool player_patterns_enabled; /* not silenced by a cut-scene, etc. */
extern u8  current_note;         /* NOTE_MI … NOTE_DO or NOTE_NONE      */
extern u8  noteQueue[4];    // Queue of notes played by the player (up to 4)

typedef struct
{
    /* identity & availability */
    u16  id;      // PATTERN_...                       
    bool enabled; // true if available for the character
    u8   notes[4]; // notes in the pattern (always 4)
    u16  baseDuration; // duration of the effect (in frames)

    /* callbacks */
    bool (*canUse)(void); // can the player use this pattern?
    void (*launch)(void); // called when the pattern is launched
    bool (*update)(void); // called every frame (true = finished)

    Sprite* icon; // HUD / menu icon (nullable)
} PlayerPattern;

typedef struct
{
    u16  id;  // PATTERN_EN_...
    u8   notes[4]; // notes in the pattern (max 4)
    u8   noteCount; // number of notes in the pattern

    /* timing */
    u16  baseDuration;   // frames the effect lasts
    u16  rechargeFrames;   // frames before the pattern can be used again

    bool enabled; // true if available for the enemy

    /* callbacks */
    void (*launch)(u8 enemyId); // called when the pattern is launched
    bool (*update)(u8 enemyId); // called every frame (true = finished)

    /* counter-spell support */
    bool counterable; // can the player counter this pattern?
    void (*onCounter)(u8 enemyId); // NULL if not counterable
} EnemyPattern;

/* -------------------------------------------------------------------
 * Static tables
 * ------------------------------------------------------------------*/
extern PlayerPattern playerPatterns[MAX_PLAYER_PATTERNS];
extern EnemyPattern  enemyPatterns[MAX_ENEMIES][MAX_PATTERN_ENEMY];

// Initialization
void init_player_patterns(void);              // Initialize graphics, sounds and pattern data

// Player side
void launch_player_pattern(u16 patternId);    // Activate pattern
bool update_player_pattern(void);             // Called every frame (true = finished)
void activate_spell(u16 npattern);          // Play spell animation and sound
bool pattern_player_add_note(u8 noteCode);     // player presses a note
void reset_note_queue(void);                // Reset the note queue and player notes count
void cancel_player_pattern(void); // Cancel the current player pattern (e.g. if the player wants to stop playing)

// Enemy side
void launch_enemy_pattern(u8 enemySlot, u16 patternSlot); // Launch an enemy pattern
bool update_enemy_pattern(u8 enemySlot);      // Called every frame (true = finished)
void pattern_enemy_add_note(u8 enemySlot, u8 noteCode); // Add a note to the enemy pattern
void pattern_enemy_clear_notes(void); // Clear all enemy notes
void init_enemy_patterns(u8 enemyId); // Initialize enemy patterns for a specific enemy slot
void cancel_enemy_pattern(u8 enemyId); // Reset enemy state after a pattern is finished

// Note validation
u16 validate_pattern(const u8 notes[4], bool* reversed); // Cancel an enemy pattern (e.g. if the player counters it)

#endif // PATTERNS_H
