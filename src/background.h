#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

// Backgrounds
extern Map *background_BGA;
extern Map *background_BGB;
extern u32 offset_BGA;
extern u32 offset_BGB;

// Background scroll modes
extern u8 background_scroll_mode; // Scroll modes (BG_SCRL_*)
extern u8 scroll_speed; // Scroll speed (each mode uses it in a way)
extern bool player_scroll_active; // Can you scroll ?
extern u16 background_width; // Background width

#define BG_SCRL_AUTO_RIGHT   00 // background scrolls in auto mode to the right (updated every frame)
#define BG_SCRL_AUTO_LEFT    01 // background scrolls in auto mode to the left (updated every frame)
#define BG_SCRL_USER_RIGHT   10 // background scrolls as the player walks - user starts in the left, and advances to the right
#define BG_SCRL_USER_LEFT    11 // background scrolls as the player walks - user starts in the right, and advances to the left

// Screen limits
extern u16 x_limit_min; // Minimum x position (if there's scroll, player scrolls at that point)
extern u16 x_limit_max; // Maximum x position (if there's scroll, player scrolls at that point)
extern u16 y_limit_min; // Minimum y position
extern u16 y_limit_max; // Maximum y position

#define X_OUT_OF_BOUNDS 9999 // Value to return when x coordinate is out of bounds

void update_bg(bool player_moved); // Update background and do the scroll
void set_limits(u16 x_min, u16 y_min, u16 x_max, u16 y_max); // Set background limits
void scroll_background(s16 dx); // Scroll the background if the character is at the screen edge. This function is called when the character can't move but the background can scroll.
u16 get_x_in_screen(u16 x_in_background, u8 width); // Get screen X coordinate from background X coordinate

#endif