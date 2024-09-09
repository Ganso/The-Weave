#include <genesis.h>
#include "globals.h"

// Wait for N seconds
void wait_seconds(int sec)
{
    u16 num_ticks=0;
    u16 max_ticks=sec*SCREEN_FPS; // Ticks
    while (num_ticks<max_ticks)
    {
        next_frame(false);
        num_ticks++;
    }

}

// Wait for next frame and do each-frame actions, including interactive actions if selected
void next_frame(bool interactive)
{
    if (interactive==true) {
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
    }

    // Screen related actions
    update_bg(true); // First time we print the background, assume the player moved so it's fully painted

    // Sprites related actions
    update_sprites_depth();
    SPR_update();

    // Items related actions
    check_items_visibility();

    // Create an RNG seed depending of the number of frames
    frame_counter++;

    // Wait for next frame
    SYS_doVBlankProcess();
}

u16 calc_ticks(u16 milliseconds) // Translate millisecons to ticks
{
    if (SCREEN_FPS == 50) {
        // 50 FPS: 1 tick ≈ 20 ms
        return (milliseconds / 20);
    } else if (SCREEN_FPS == 60) {
        // 60 FPS: 1 tick ≈ 16.67 ms
        return ((milliseconds<<2) + (milliseconds<<1)) / 100;
    } else return ((milliseconds*1000)/SCREEN_FPS);
}