#include <genesis.h>
#include "globals.h"

/**
 * Main function to check and handle joystick input.
 * This function is called every frame to process player input.
 */
void joy_check(void)
{
    // Read the current state of the joystick
    u16 joy_value = JOY_readJoypad(JOY_ALL);

    // Only process movement and action buttons if movement is allowed
    if (movement_active) {
        // Handle movement only if no note is currently playing
        if (note_playing == NOTE_NONE) {
            handle_movement(joy_value);
        }
        // Always check action buttons, even if a note is playing
        handle_action_buttons(joy_value);
    }

    // Always check for pause button, regardless of game state
    handle_pause_button(joy_value);
}

/**
 * Handle character movement based on joystick input.
 * This function checks for directional button presses and moves the character accordingly.
 * 
 * @param joy_value The current state of the joystick
 */
void handle_movement(u16 joy_value)
{
    bool moved = false;
    s16 dx = 0, dy = 0;

    // Check for left/right movement
    if (joy_value & BUTTON_LEFT) {
        dx = -1;
    } else if (joy_value & BUTTON_RIGHT) {
        dx = 1;
    }

    // Check for up/down movement
    if (joy_value & BUTTON_UP) {
        dy = -1;
    } else if (joy_value & BUTTON_DOWN) {
        dy = 1;
    }

    // If any direction button was pressed, attempt to move the character
    if (dx != 0 || dy != 0) {
        handle_character_movement(dx, dy);
        moved = true;
    }

    // Update the character's animation based on whether it moved or not
    update_character_animation(moved);
}

// Move the character if there's no collision and it's within the screen limits. This function also handles background scrolling if the character is at the screen edge.
void handle_character_movement(s16 dx, s16 dy)
{
    // dx Horizontal movement (-1 for left, 1 for right, 0 for no horizontal movement)
    // dy Vertical movement (-1 for up, 1 for down, 0 for no vertical movement)

    // Calculate the new position
    s16 new_x = obj_character[active_character].x + dx;
    s16 new_y = obj_character[active_character].y + dy;
    u8 player_y_size = obj_character[active_character].y_size;

    // Check for collision at the new position
    if ((detect_char_enemy_collision(active_character, new_x, new_y) != ENEMY_NONE) || (detect_char_item_collision(active_character, new_x, new_y) != ITEM_NONE)) {
        return; // Collision detected, don't move
    }

    // Handle horizontal movement
    if (dx != 0) {
        if ( (background_scroll_mode == BG_SCRL_USER_RIGHT || background_scroll_mode == BG_SCRL_USER_LEFT) && (new_x < x_limit_min || new_x > x_limit_max) ) { // Player is trying to move outside screen boundaries, and scroll mode is user dependant
            scroll_background(dx); // Try to scroll
        }
        else if (new_x >= x_limit_min && new_x <= x_limit_max) { // In any other circunstance, if new x is between x_limit_min and x_limit_max, update x in object
            obj_character[active_character].x = new_x;
            obj_character[active_character].flipH = (dx < 0); // Flip character sprite if moving left
        }
        // If new_x is outside boundaries and not in scrolling mode, do nothing
    }

    // Handle vertical movement
    if (dy != 0 && new_y + player_y_size >= y_limit_min && new_y + player_y_size <= y_limit_max) {
        obj_character[active_character].y = new_y;
    }
}

/**
 * Update the character's animation based on whether it moved or not.
 * 
 * @param moved Whether the character moved in this frame
 */
void update_character_animation(bool moved)
{
    if (moved) {
        obj_character[active_character].animation = ANIM_WALK;
        update_character(active_character);
    } else if (obj_character[active_character].animation == ANIM_WALK) {
        anim_character(active_character, ANIM_IDLE); // Stop walking animation if character stopped moving
    }
}

/**
 * Handle action button inputs (A, B, C, X, Y, Z).
 * Each button plays a different musical note.
 * 
 * @param joy_value The current state of the joystick
 */
void handle_action_buttons(u16 joy_value)
{
    if (joy_value & BUTTON_A) play_note(NOTE_MI);
    if (joy_value & BUTTON_B) play_note(NOTE_FA);
    if (joy_value & BUTTON_C) play_note(NOTE_SOL);
    if (joy_value & BUTTON_X) play_note(NOTE_LA);
    if (joy_value & BUTTON_Y) play_note(NOTE_SI);
    if (joy_value & BUTTON_Z) play_note(NOTE_DO);

    update_action_animation();
}

/**
 * Update the character's animation when playing a musical note.
 */
void update_action_animation(void)
{
    if (note_playing != NOTE_NONE && obj_character[active_character].animation != ANIM_ACTION) {
        anim_character(active_character, ANIM_ACTION); // Start action animation when playing a note
    } else if (note_playing == NOTE_NONE && obj_character[active_character].animation == ANIM_ACTION) {
        anim_character(active_character, ANIM_IDLE); // Return to idle animation when note stops
    }
}

/**
 * Handle the pause button input.
 * When the START button is pressed, the game pauses.
 * 
 * @param joy_value The current state of the joystick
 */
void handle_pause_button(u16 joy_value)
{
    if (joy_value & BUTTON_START) {
        // Wait for the START button to be released
        while (joy_value & BUTTON_START) {
            joy_value = JOY_readJoypad(JOY_ALL);
            SYS_doVBlankProcess();
        }
        pause_screen(); // Call the pause screen function
    }
}