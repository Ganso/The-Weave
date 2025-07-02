#include "globals.h"

Map *background_BGA;              // Foreground layer tilemap
Map *background_BGB;              // Background layer tilemap
u32 offset_BGA;                   // Foreground scroll offset
u32 offset_BGB;                   // Background scroll offset
u8 background_scroll_mode;        // Current scrolling behavior mode
u8 scroll_speed;                  // Parallax scroll speed divider
bool player_scroll_active;        // Whether player can trigger scrolling
u16 background_width;             // Total width of background in pixels
u16 x_limit_min;                  // Minimum X position when no scroll
u16 x_limit_max;                  // Maximum X position when no scroll
u16 y_limit_min;                  // Minimum Y position when no scroll
u16 y_limit_max;                  // Maximum Y position when no scroll

void update_bg(bool player_moved)    // Update background scroll positions based on movement
{
    // Front background layer
    if (player_moved) MAP_scrollTo(background_BGA, offset_BGA, 0); // Scroll background A
    
    // Back background layer
    if (background_BGB!=NULL) {
        if (background_scroll_mode==BG_SCRL_USER_LEFT || background_scroll_mode==BG_SCRL_USER_RIGHT) {
            if (player_moved) {
                offset_BGB = offset_BGA >> scroll_speed; // Scroll by user movement
                MAP_scrollTo(background_BGB, offset_BGB, 0); // Scroll background B
            }
        }
        else { // Auto scroll
            if (background_scroll_mode==BG_SCRL_AUTO_LEFT) offset_BGB++;
            else offset_BGB--;
            MAP_scrollTo(background_BGB, offset_BGB >> scroll_speed, 0); // Scroll background B
        }
    }
}

void set_limits(u16 x_min, u16 y_min, u16 x_max, u16 y_max)    // Set movement boundary limits for characters
{
    x_limit_min=x_min;
    y_limit_min=y_min;
    x_limit_max=x_max;
    y_limit_max=y_max;
}

void scroll_background(s16 dx)    // Handle background scrolling when character reaches screen edge
{
    if (player_scroll_active) { // Can player scroll?
        if (background_scroll_mode == BG_SCRL_USER_RIGHT || background_scroll_mode == BG_SCRL_USER_LEFT) { // Scrolling mode is user dependent?
            s16 new_offset = (s16)offset_BGA + dx;
            s16 max_offset = (s16)background_width - SCREEN_WIDTH;
            if (new_offset >= 0 && new_offset <= max_offset) { // New scroll offset is inside background width boundaries?
                offset_BGA = (u32)new_offset; // Change offset
                update_bg(true);
                // Move following characters to the left/right accordingly
                for (u16 nchar=0; nchar<MAX_CHR; nchar ++) {
                    if (obj_character[nchar].follows_character==true) {
                        if (FASTFIX32_TO_INT(obj_character[nchar].x)>-20) {
                            obj_character[nchar].x -= FASTFIX32_FROM_INT(dx);
                            update_character(nchar);
                        }
                    }
                }
            }
        }
    }
}

u16 get_x_in_screen(u16 x_in_background, u8 width)    // Convert background X coordinate to screen space
{
    s16 x_in_screen = x_in_background - offset_BGA;
    
    // Allow sprite to be partially visible at screen edges
    // A sprite should be visible if any part of it is on screen
    if (x_in_screen < -(s16)width || x_in_screen >= SCREEN_WIDTH) {
        return X_OUT_OF_BOUNDS;
    }
    
    return (u16)x_in_screen;
}
