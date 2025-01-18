#ifndef _INTERFACE_H_
#define _INTERFACE_H_

#include "globals.h"

/**
 * @brief Structure to store sprite visibility state
 * Used for pause screen to save/restore sprite states
 */
typedef struct {
    Sprite* sprite;              // Sprite pointer
    SpriteVisibility visibility; // Current visibility
} SpriteState;

/**
 * @brief Interface sprite pointers
 */
// Face backgrounds
extern Sprite *spr_face_left;    // Left face background
extern Sprite *spr_face_right;   // Right face background

// Button indicators
extern Sprite *spr_int_button_A; // A button indicator

// Note rod elements
extern Map *map_int_rod;         // Rod background
extern Sprite *spr_int_rod_1;    // Rod note 1 (MI)
extern Sprite *spr_int_rod_2;    // Rod note 2 (FA)
extern Sprite *spr_int_rod_3;    // Rod note 3 (SOL)
extern Sprite *spr_int_rod_4;    // Rod note 4 (LA)
extern Sprite *spr_int_rod_5;    // Rod note 5 (SI)
extern Sprite *spr_int_rod_6;    // Rod note 6 (DO)

// Enemy note indicators
extern Sprite *spr_int_enemy_rod_1;  // Enemy note 1 (MI)
extern Sprite *spr_int_enemy_rod_2;  // Enemy note 2 (FA)
extern Sprite *spr_int_enemy_rod_3;  // Enemy note 3 (SOL)
extern Sprite *spr_int_enemy_rod_4;  // Enemy note 4 (LA)
extern Sprite *spr_int_enemy_rod_5;  // Enemy note 5 (SI)
extern Sprite *spr_int_enemy_rod_6;  // Enemy note 6 (DO)

// Pentagram note indicators
extern Sprite *spr_int_pentagram_1;  // Pentagram note 1 (MI)
extern Sprite *spr_int_pentagram_2;  // Pentagram note 2 (FA)
extern Sprite *spr_int_pentagram_3;  // Pentagram note 3 (SOL)
extern Sprite *spr_int_pentagram_4;  // Pentagram note 4 (LA)
extern Sprite *spr_int_pentagram_5;  // Pentagram note 5 (SI)
extern Sprite *spr_int_pentagram_6;  // Pentagram note 6 (DO)

// Status indicators
extern Sprite *spr_int_life_counter; // Life counter sprite

// Pause screen elements
extern Sprite *spr_pause_icon[MAX_PAUSE_ICONS];         // Pause menu icons
extern Sprite *spr_pattern_list_note[MAX_PATTERN_NOTES]; // Pattern list notes

// Interface state
extern bool interface_active;    // Whether interface is visible

/**
 * @brief Show/hide the bottom interface
 * @param visible Whether to show interface
 */
void show_or_hide_interface(bool visible);

/**
 * @brief Show/hide note indicator
 * @param nnote Note number (1-6:MI-DO)
 * @param visible Whether to show note
 */
void show_note(u8 nnote, bool visible);

/**
 * @brief Hide all rod note indicators
 */
void hide_rod_icons(void);

/**
 * @brief Hide all pentagram note indicators
 */
void hide_pentagram_icons(void);

/**
 * @brief Hide all pattern icons
 */
void hide_pattern_icons(void);

/**
 * @brief Show/hide pattern icon
 * @param npattern Pattern ID
 * @param show Whether to show icon
 * @param priority Icon priority
 */
void show_pattern_icon(u16 npattern, bool show, bool priority);

/**
 * @brief Restore sprite visibility states
 * @param states Array of sprite states
 * @param count Number of states
 */
void restoreSpritesVisibility(SpriteState* states, u16 count);

/**
 * @brief Hide all sprites and save states
 * @param count Output parameter for number of states
 * @return Array of saved sprite states
 */
SpriteState* hideAllSprites(u16* count);

/**
 * @brief Show pause/state screen
 */
void pause_screen(void);

/**
 * @brief Show/hide pattern list in pause screen
 * @param show Whether to show list
 * @param active_pattern Currently active pattern
 */
void show_pause_pattern_list(bool show, u8 active_pattern);

/**
 * @brief Show note in pause pattern list
 * @param npattern Pattern ID
 * @param nnote Note number (1-6:MI-DO)
 * @param show Whether to show note
 */
void show_note_in_pause_pattern_list(u8 npattern, u8 nnote, bool show);

/**
 * @brief Show icon in pause menu list
 * @param npattern Pattern ID
 * @param nicon Icon index
 * @param x X position
 * @param show Whether to show icon
 * @param priority Icon priority
 */
void show_icon_in_pause_list(u16 npattern, u8 nicon, u16 x, bool show, bool priority);

#endif // _INTERFACE_H_
