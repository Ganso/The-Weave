#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include "globals.h"

/**
 * @brief Background layer maps and offsets
 */
extern Map *background_BGA;              // Foreground layer tilemap
extern Map *background_BGB;              // Background layer tilemap
extern u32 offset_BGA;                   // Foreground scroll offset
extern u32 offset_BGB;                   // Background scroll offset

/**
 * @brief Background scroll settings
 */
extern u8 background_scroll_mode;        // Scroll mode (BG_SCRL_*)
extern u8 scroll_speed;                  // Scroll speed divider
extern bool player_scroll_active;        // Whether player can trigger scrolling
extern u16 background_width;             // Total background width

/**
 * @brief Movement boundary limits
 */
extern u16 x_limit_min;                  // Minimum X position
extern u16 x_limit_max;                  // Maximum X position
extern u16 y_limit_min;                  // Minimum Y position
extern u16 y_limit_max;                  // Maximum Y position

/**
 * @brief Update background scroll positions
 * @param player_moved Whether player moved this frame
 */
void update_bg(bool player_moved);

/**
 * @brief Set movement boundary limits
 * @param x_min Minimum X position
 * @param y_min Minimum Y position
 * @param x_max Maximum X position
 * @param y_max Maximum Y position
 */
void set_limits(u16 x_min, u16 y_min, u16 x_max, u16 y_max);

/**
 * @brief Handle background scrolling at screen edge
 * @param dx Amount to scroll horizontally
 */
void scroll_background(s16 dx);

/**
 * @brief Convert background X coordinate to screen space
 * @param x_in_background X position in background coordinates
 * @param width Width of object
 * @return Screen X coordinate or X_OUT_OF_BOUNDS if not visible
 */
u16 get_x_in_screen(u16 x_in_background, u8 width);

#endif // _BACKGROUND_H_
