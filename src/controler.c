#include <genesis.h>
#include "globals.h"

// Check for joystick input
void joy_check(void)
{
    u16 value = JOY_readJoypad(JOY_ALL);

    // Movements
    if (movement_active == true) {
        if (note_playing == NOTE_NONE) // Only move if no note is playing
        {
            joy_check_movement(value);
        }

        // Action buttons
        joy_check_buttons(value);
    }

    // Pause / State button
    if (value & BUTTON_START)
    {
        while (value & BUTTON_START) { // First, wait until released
            value = JOY_readJoypad(JOY_ALL);
            SYS_doVBlankProcess();
        }
        pause_screen();
    }
}

// Check for movement joystick input
void joy_check_movement(u16 value)
{
    bool moved = false;

    if (value & BUTTON_LEFT)
    {
        if (detect_char_collision(active_character, obj_character[active_character].x-1, obj_character[active_character].y) != ENEMY_NONE) return; // Collision !
        if (obj_character[active_character].x > x_limit_min) {
            obj_character[active_character].x--;
            obj_character[active_character].flipH = true;
            moved = true;
        }
        else { // Minimum X position
            if (background_scroll_mode == BG_SCRL_USER_RIGHT && offset_BGA > 0 && player_scroll_active == true) {
                offset_BGA--;
                offset_BGB = offset_BGA >> scroll_speed;
                MAP_scrollTo(background_BGA, offset_BGA, 0);
                if (background_BGB != NULL) MAP_scrollTo(background_BGB, offset_BGB, 0);
                moved = true;
            }
        }
    }

    if (value & BUTTON_RIGHT)
    {
        if (detect_char_collision(active_character, obj_character[active_character].x+1, obj_character[active_character].y) != ENEMY_NONE) return; // Collision !
        if (obj_character[active_character].x < x_limit_max) {
            obj_character[active_character].x++;
            obj_character[active_character].flipH = false;
            moved = true;
        }
        else { // Maximum X position
            if (background_scroll_mode == BG_SCRL_USER_RIGHT && player_scroll_active == true) {
                offset_BGA++;
                offset_BGB = offset_BGA >> scroll_speed;
                MAP_scrollTo(background_BGA, offset_BGA, 0);
                if (background_BGB != NULL) MAP_scrollTo(background_BGB, offset_BGB, 0);
                moved = true;
            }
        }
    }

    if (value & BUTTON_UP)
    {
        if (detect_char_collision(active_character, obj_character[active_character].x, obj_character[active_character].y-1) != ENEMY_NONE) return; // Collision !
        if (obj_character[active_character].y > y_limit_min) {
            obj_character[active_character].y--;
            moved = true;
        }
    }

    if (value & BUTTON_DOWN)
    {
        if (detect_char_collision(active_character, obj_character[active_character].x, obj_character[active_character].y+1) != ENEMY_NONE) return; // Collision !
        if (obj_character[active_character].y < y_limit_max) {
            obj_character[active_character].y++;
            moved = true;
        }
    }

    if (moved) { // Player moved
        obj_character[active_character].animation = ANIM_WALK; // Animate
        update_character(active_character); // Update
    }
    else if (obj_character[active_character].animation == ANIM_WALK) anim_character(active_character, ANIM_IDLE); // Not moving -> Stop walking animation
}

// Check for action button inputs
void joy_check_buttons(u16 value)
{
    if (value & BUTTON_A)
    {
        play_note(NOTE_MI);
    }

    if (value & BUTTON_B)
    {
        play_note(NOTE_FA);
    }

    if (value & BUTTON_C)
    {
        play_note(NOTE_SOL);
    }

    if (value & BUTTON_X)
    {
        play_note(NOTE_LA);
    }

    if (value & BUTTON_Y)
    {
        play_note(NOTE_SI);
    }

    if (value & BUTTON_Z)
    {
        play_note(NOTE_DO);
    }

    // If we are playing a note, change animation
    if (note_playing != NOTE_NONE && obj_character[active_character].animation != ANIM_ACTION) anim_character(active_character, ANIM_ACTION); // Note start
    else if (note_playing == NOTE_NONE && obj_character[active_character].animation == ANIM_ACTION) anim_character(active_character, ANIM_IDLE); // Note end
}