#ifndef _GAME_CONSTANTS_H_
#define _GAME_CONSTANTS_H_

// Pattern system constants
#define MAX_PATTERN_ENEMY 2
#define MAX_PATTERNS 4

// Pattern timing constants
#define MAX_NOTE_PLAYING_TIME  500  // Note playing time in milliseconds
#define MAX_PATTERN_WAIT_TIME 2000  // Time to wait for next note before cancelling
#define MAX_ATTACK_NOTE_PLAYING_TIME  500  // Attack note playing time
#define MAX_TIME_AFTER_ATTACK        1000  // Time to stay in finished state
#define MAX_EFFECT_TIME_ELECTRIC     1600  // Electric pattern effect duration
#define MAX_EFFECT_TIME_BITE         1400  // Bite pattern effect duration

// Pattern types
#define PTRN_NONE         254
#define PTRN_ELECTRIC      0   // Electricity spell
#define PTRN_HIDE          1   // Hide spell
#define PTRN_OPEN          2   // Open spell
#define PTRN_SLEEP         3   // Sleep spell

// Enemy pattern types
#define PTRN_EN_NONE         254
#define PTRN_EN_ELECTIC      0   // Enemy electricity spell
#define PTRN_EN_BITE         1   // Enemy bite spell

// Note constants
#define NOTE_NONE 0
#define NOTE_MI   1
#define NOTE_FA   2
#define NOTE_SOL  3
#define NOTE_LA   4
#define NOTE_SI   5
#define NOTE_DO   6

// Enemy system constants
#define MAX_ENEMIES 4
#define MAX_ENEMY_CLASSES 2
#define ENEMY_NONE 254

// Enemy class IDs
#define ENEMY_CLS_BADBOBBIN    0
#define ENEMY_CLS_3HEADMONKEY  1

// Character system constants
#define MAX_CHR       4
#define CHR_NONE      254

// Character IDs
#define CHR_linus     0
#define CHR_clio      1
#define CHR_xander    2
#define CHR_swan      3

// Face system constants
#define MAX_FACE      4
#define FACE_linus    0
#define FACE_clio     1
#define FACE_xander   2
#define FACE_swan     3
#define FACE_none     250

// Face positioning
#define SIDE_left     true
#define SIDE_right    false
#define SIDE_none     true

// Animation constants
#define ANIM_IDLE 0
#define ANIM_WALK 1
#define ANIM_MAGIC 2
#define ANIM_ACTION 3

// State constants
#define STATE_IDLE 0
#define STATE_WALKING 1
#define STATE_PLAYING_NOTE 2
#define STATE_PATTERN_CHECK 3
#define STATE_PATTERN_EFFECT 4
#define STATE_PATTERN_EFFECT_FINISH 5
#define STATE_PATTERN_FINISHED 6
#define STATE_ATTACK_FINISHED 7
#define STATE_FOLLOWING 8

// Background scroll modes
#define BG_SCRL_AUTO_RIGHT   00  // Background scrolls right automatically
#define BG_SCRL_AUTO_LEFT    01  // Background scrolls left automatically
#define BG_SCRL_USER_RIGHT   10  // User scrolls right (starts from left)
#define BG_SCRL_USER_LEFT    11  // User scrolls left (starts from right)

// Screen constants
#define X_OUT_OF_BOUNDS 9999     // Value for out of bounds X coordinate

// Dialog timing constants
#define DEFAULT_TALK_TIME   10    // Default dialog duration (10 seconds)
#define DEFAULT_CHOICE_TIME 30    // Default choice duration (30 seconds)

// Interface constants
#define MAX_PAUSE_ICONS 5         // Number of pause menu icons
#define MAX_PATTERN_NOTES 4       // Max notes in pattern list

// Language constants
#define NUM_LANGUAGES 2           // Number of supported languages
#define MAX_CHOICES 4             // Maximum choices in dialog

// Dialog IDs
#define SYSTEM_DIALOG   0         // System messages
#define ACT1_DIALOG1    1         // Act 1 dialog sequences
#define ACT1_DIALOG2    2
#define ACT1_DIALOG3    3
#define ACT1_DIALOG4    4

// Choice IDs
#define ACT1_CHOICE1    0         // Act 1 choice sequences

// Sound system constants
#define XGM_VERSION 2             // XGM or XGM2 sound driver version

// Collision system constants
#define MAX_INTERACTIVE_DISTANCE 20  // Distance for item interaction
#define MAX_COLLISIONS 30           // Max collision retries before forcing movement

// Item system constants
#define MAX_ITEMS 10               // Maximum number of items
#define ITEM_NONE 254             // No item selected/found
#define COLLISION_DEFAULT 9999     // Default collision value

#endif // _GAME_CONSTANTS_H_
