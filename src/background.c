#include <genesis.h>
#include "globals.h"

// Update_background
void update_bg(bool player_moved)
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

// Set background limits
void set_limits(u16 x_min, u16 y_min, u16 x_max, u16 y_max)
{
    x_limit_min=x_min;
    y_limit_min=y_min;
    x_limit_max=x_max;
    y_limit_max=y_max;
}

// Scroll the background if the character is at the screen edge. This function is called when the character can't move but the background can scroll.
void scroll_background(s16 dx)
{
    if (player_scroll_active) { // Can player scroll?
        if (background_scroll_mode == BG_SCRL_USER_RIGHT || background_scroll_mode == BG_SCRL_USER_LEFT) { // Scrolling mode is user dependant ?
            if (offset_BGA+dx>0 && ((s16) offset_BGA+dx)<((s16) background_width-SCREEN_WIDTH)) { // New scroll offset is inside background width boundries?
                offset_BGA+=dx; // Change offste
                update_bg(true);
            }
        }
    }
}

// Get screen X coordinate from background X coordinate
u16 get_x_in_screen(u16 x_in_background, u8 width)
{
    s16 x_in_screen = x_in_background - offset_BGA;
    
    if (x_in_screen < -width || x_in_screen >= SCREEN_WIDTH) {
        return X_OUT_OF_BOUNDS;
    }
    
    return (u16)x_in_screen;
}