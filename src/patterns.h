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
#define MIN_TIME_BETWEEN_NOTES SCREEN_FPS/4    // Minimum ticks between consecutive notes to prevent too rapid input
#define MIN_TIME_BETWEEN_PATTERNS SCREEN_FPS/4 // Minimum ticks between consecutive patterns

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
void initPlayerPatterns(void);              // Initialize graphics, sounds and pattern data

// Player side
void launchPlayerPattern(u16 patternId);    // Activate pattern
bool updatePlayerPattern(void);             // Called every frame (true = finished)
void activate_spell(u16 npattern);          // Play spell animation and sound
bool patternPlayerAddNote(u8 noteCode);     // player presses a note
void reset_note_queue(void);                // Reset the note queue and player notes count
void cancelPlayerPattern(void); // Cancel the current player pattern (e.g. if the player wants to stop playing)

// Enemy side
void launchEnemyPattern(u8 enemySlot, u16 patternSlot);
bool updateEnemyPattern(u8 enemySlot);      // Called every frame (true = finished)
void patternEnemyAddNote(u8 enemySlot, u8 noteCode); // enemy plays a note
void initEnemyPatterns(u8 enemyId); // Initialize enemy patterns for a specific enemy slot

// Note validation
u16 validatePattern(const u8 notes[4], bool* reversed); // Returns PATTERN_PLAYER_NONE if invalid

#endif // PATTERNS_H