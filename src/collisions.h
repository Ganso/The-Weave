#ifndef _COLLISIONS_H_
#define _COLLISIONS_H_

#include "globals.h"

/**
 * @brief Collision retry counter
 * Tracks number of collision checks before forcing movement
 */
extern u8 num_colls;

/**
 * @brief Calculate distance between two characters
 * @param char1 First character ID
 * @param x1 Target X position
 * @param y1 Target Y position
 * @param char2 Second character ID
 * @return Distance in pixels
 */
u16 char_distance(u16 char1, s16 x1, u8 y1, u16 char2);

/**
 * @brief Calculate distance to item
 * @param nitem Item ID
 * @param x Target X position
 * @param y Target Y position
 * @return Distance to item's collision box center
 */
u16 item_distance(u16 nitem, u16 x, u8 y);

/**
 * @brief Check character-character collisions
 * @param nchar Character ID to check
 * @param x Target X position
 * @param y Target Y position
 * @return ID of colliding character or CHR_NONE
 */
u16 detect_char_char_collision(u16 nchar, u16 x, u8 y);

/**
 * @brief Check character-item collisions
 * @param nchar Character ID to check
 * @param x Target X position
 * @param y Target Y position
 * @return ID of colliding item or CHR_NONE
 */
u16 detect_char_item_collision(u16 nchar, u16 x, u8 y);

/**
 * @brief Check character-enemy collisions
 * @param nchar Character ID to check
 * @param x Target X position
 * @param y Target Y position
 * @return ID of colliding enemy or ENEMY_NONE
 */
u16 detect_char_enemy_collision(u16 nchar, u16 x, u8 y);

/**
 * @brief Check enemy-character collisions
 * @param nenemy Enemy ID to check
 * @param x Target X position
 * @param y Target Y position
 * @return ID of colliding character or CHR_NONE
 */
u16 detect_enemy_char_collision(u16 nenemy, u16 x, u8 y);

#endif // _COLLISIONS_H_
