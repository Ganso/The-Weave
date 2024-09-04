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
    // Check controller
    if (movement_active==true) joy_check();

    // Pattern related actions 
    check_note();
    check_pattern_effect();

    // Combat related actions
    if (is_combat_active==true) {
        check_enemy_pattern();
        approach_enemies();
    }

    // Screen related actions
    update_bg(true); // First time we print the background, assume the player moved so it's fully painted

    // Sprites related actions
    update_sprites_depth();
    SPR_update();

    // Create a random seed depending of the number of frames
    random_seed++;

    // Wait for next frame
    SYS_doVBlankProcess();
}
