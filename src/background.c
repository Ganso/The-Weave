#include <genesis.h>
#include "globals.h"

// Update_background
void update_bg(void)
{
    if (background_scroll_mode==BG_SCRL_AUTO_LEFT) {
        if (background_BGB!=NULL) {
            MAP_scrollTo(background_BGB, offset_BGB>>scroll_speed, 0);
            offset_BGB++;
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

/**
 * Scroll the background if the character is at the screen edge.
 * This function is called when the character can't move but the background can scroll.
 * 
 * @param dx The direction to scroll (-1 for left, 1 for right)
 */
void scroll_background(s16 dx)
{
    if (background_scroll_mode == BG_SCRL_USER_RIGHT && player_scroll_active) {
        // Only scroll if moving right, or moving left and not at the left edge
        if ((dx < 0 && offset_BGA > 0) || dx > 0) {
            offset_BGA += dx;
            offset_BGB = offset_BGA >> scroll_speed; // Parallax scrolling for background B
            MAP_scrollTo(background_BGA, offset_BGA, 0);
            if (background_BGB != NULL) {
                MAP_scrollTo(background_BGB, offset_BGB, 0);
            }
        }
    }
}