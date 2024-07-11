#include <genesis.h>
#include "globals.h"

// Check for joystick input
void joy_check(void)
{
    u16 value = JOY_readJoypad(JOY_ALL);
    bool moved=false;

    if (value & BUTTON_LEFT )
        {
            if (obj_character[active_character].x>x_limit_min) {
                obj_character[active_character].x--;
                obj_character[active_character].flipH=true;
                moved=true;
            }
        }

    if (value & BUTTON_RIGHT)
        {
            if (obj_character[active_character].x<x_limit_max) {
                obj_character[active_character].x++;
                obj_character[active_character].flipH=false;
                moved=true;
                }
            else { // Maximum X position
                if (background_scroll_mode==BG_SCRL_USER_RIGHT) {
                    offset_BGA++;
                    offset_BGB=offset_BGA>>2;
                    MAP_scrollTo(background_BGA, offset_BGA, 0);
                    MAP_scrollTo(background_BGB, offset_BGB, 0);
                }
            }
        }

    if (value & BUTTON_UP)
        {
            if (obj_character[active_character].y>y_limit_min) {
                obj_character[active_character].y--;
                moved=true;
            }
        }
    if (value & BUTTON_DOWN)
        {
            if (obj_character[active_character].y<y_limit_max) {
                obj_character[active_character].y++;
                moved=true;
            }
        }

    if (moved) update_character(active_character);

}
