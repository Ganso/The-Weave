#include "globals.h"

u16 tile_ind;        // Current tile index for VDP tile loading
u16 frame_counter;   // Counter incremented each frame for RNG and timing
u8 current_act;      // Current game act number
u8 current_scene;    // Current scene number within act
u8 SCREEN_FPS;      // Screen refresh rate (50 for PAL, 60 for NTSC)

void wait_seconds(int sec)    // Pause execution for specified number of seconds
{
    u16 num_ticks=0;
    u16 max_ticks=sec*SCREEN_FPS; // Ticks
    while (num_ticks<max_ticks)
    {
        next_frame(false);
        num_ticks++;
    }
}

void next_frame(bool interactive)    // Process next frame with optional interactive updates (movement, combat, etc)
{
    if (interactive==true) {
        // Check controller
        if (movement_active==true) {
            joy_check();
            if (player_patterns_enabled) check_pattern_status();
        }

        // Approach other characters
        approach_characters();

        // Combat related actions
        update_combat();
    }

    // Screen related actions
    update_bg(true); // First time we print the background, assume the player moved so it's fully painted

    // Items related actions
    check_items_visibility();

    // Sprites related actions
    update_character_animations();
    update_enemy_animations();
    update_sprites_depth();
    SPR_update();

    // Create an RNG seed deTODO of the number of frames
    frame_counter++;
    
    // Wait for next frame
    SYS_doVBlankProcess();
}

// Convert milliseconds to system ticks based on screen refresh rate
u16 calc_ticks(u16 milliseconds)    
{
    if (SCREEN_FPS == 50) {
        // 50 FPS: 1 tick ≈ 20 ms
        return (milliseconds / 20);
    } else if (SCREEN_FPS == 60) {
        // 60 FPS: 1 tick ≈ 16.67 ms
        return ((milliseconds<<2) + (milliseconds<<1)) / 100;
    } else return ((milliseconds*1000)/SCREEN_FPS);
}
