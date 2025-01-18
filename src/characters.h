#ifndef _CHARACTERS_H_
#define _CHARACTERS_H_

#include "globals.h"

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

// Character entities
extern Entity obj_character[MAX_CHR];
extern Sprite *spr_chr[MAX_CHR];
extern Sprite *spr_chr_shadow[MAX_CHR];
extern u16 active_character;    // Currently controlled character
extern bool movement_active;    // Whether movement is allowed

// Face entities
extern Entity obj_face[MAX_FACE];
extern Sprite *spr_face[MAX_FACE];

/**
 * @brief Initialize a character
 * @param nchar Character ID to initialize
 */
void init_character(u16 nchar);

/**
 * @brief Release character resources
 * @param nchar Character ID to release
 */
void release_character(u16 nchar);

/**
 * @brief Initialize a face
 * @param nface Face ID to initialize
 */
void init_face(u16 nface);

/**
 * @brief Release face resources
 * @param nface Face ID to release
 */
void release_face(u16 nface);

/**
 * @brief Update character sprite properties
 * @param nchar Character ID to update
 */
void update_character(u16 nchar);

/**
 * @brief Show/hide character and shadow
 * @param nchar Character ID to show/hide
 * @param show Whether to show
 */
void show_character(u16 nchar, bool show);

/**
 * @brief Set character animation
 * @param nchar Character ID to animate
 * @param newanimation Animation ID
 */
void anim_character(u16 nchar, u8 newanimation);

/**
 * @brief Set character facing direction
 * @param nchar Character ID to update
 * @param left true to face left
 */
void look_left(u16 nchar, bool left);

/**
 * @brief Move character with animation
 * @param nchar Character ID to move
 * @param x New X position
 * @param y New Y position
 */
void move_character(u16 nchar, s16 x, s16 y);

/**
 * @brief Move character instantly
 * @param nchar Character ID to move
 * @param x New X position
 * @param y New Y position
 */
void move_character_instant(u16 nchar, s16 x, s16 y);

/**
 * @brief Update sprite depth ordering
 */
void update_sprites_depth(void);

/**
 * @brief Update character shadow position
 * @param nchar Character ID to update
 */
void update_character_shadow(u16 nchar);

/**
 * @brief Set character to follow active character
 * @param nchar Character ID to update
 * @param follow Whether to follow
 * @param follow_speed Movement speed when following
 */
void follow_active_character(u16 nchar, bool follow, u8 follow_speed);

/**
 * @brief Update following characters' positions
 */
void approach_characters(void);

#endif // _CHARACTERS_H_
