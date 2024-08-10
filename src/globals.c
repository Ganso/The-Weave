#include <genesis.h>
#include "globals.h"

// Wait for N seconds
void wait_seconds(int sec)
{
    u16 num_ticks=0;
    u16 max_ticks=sec*60; // Ticks (at 60fps)
    while (num_ticks<max_ticks)
    {
        next_frame();
        num_ticks++;
    }

}

// Wait for next frame and do each-frame actions
void next_frame(void)
{

    // Pattern related actions 
    check_note();

    // Combat related actions
    if (is_combat_active==true) check_enemy_pattern();

    // Screen related actions
    update_bg();

    // Sprites related actions
    update_sprites_depth();
    SPR_update();

    // Do some random stuff
    u16 rnd=random();

    // Wait for next frame
    SYS_doVBlankProcess();
}
