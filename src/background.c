#include <genesis.h>
#include "globals.h"

// Update_background
void update_bg(void)
{
    if (background_scroll_mode==BG_SCRL_AUTO_LEFT) {
        MAP_scrollTo(background_BGB, offset_BGB>>scroll_speed, 0);
        offset_BGB++;
    }
}

// Set background limits
void set_limits(u16 x1, u16 y1, u16 x2, u16 y2)
{
    x_limit_min=x1;
    y_limit_min=y1;
    x_limit_max=x2;
    y_limit_max=y2;
}
