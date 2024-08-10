#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

// Backgrounds
Map *background_BGA;
Map *background_BGB;
u32 offset_BGA;
u32 offset_BGB;

// Background scroll modes
u8 background_scroll_mode; // Scroll modes (BG_SCRL_*)
u8 scroll_speed; // Scroll speed (each mode uses it in a way)
bool player_scroll_active; // Can you scroll ?
#define BG_SCRL_AUTO_RIGHT   0  // auto mode (updated every frame)
#define BG_SCRL_AUTO_LEFT    1  // no use
#define BG_SCRL_USER_RIGHT   10 // user mode (updated on character walk)
#define BG_SCRL_USER_LEFT    11 // no use

// Screen limits
u16 x_limit_min; // Minimum x position
u16 x_limit_max; // Maximum x position
u16 y_limit_min; // Minimum y position
u16 y_limit_max; // Maximum y position


void update_bg(void); // Update background
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2); // Set background limits

#endif