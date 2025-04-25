#include <genesis.h>
#include "globals.h"

void joy_check(void)    // Process joystick input each frame for movement, actions, and pause
{
    // Read the current state of the joystick
    u16 joy_value = JOY_readJoypad(JOY_ALL);

    // Only process movement and action buttons if movement is allowed
    if (movement_active) {
        // Handle movement if character is in IDLE, WALKING, or during HIDE effect
        if (obj_character[active_character].state == STATE_IDLE || 
            obj_character[active_character].state == STATE_WALKING ||
            (obj_character[active_character].state == STATE_PATTERN_EFFECT && 
             player_pattern_effect_in_progress == PTRN_HIDE)) {
            handle_movement(joy_value);
        }
        // Always check action buttons, even during pattern effects
        handle_action_buttons(joy_value);
    }

    // Always check for pause button, regardless of game state
    handle_pause_button(joy_value);
}

void handle_movement(u16 joy_value)    // Process directional inputs and update character movement state
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

    // Update the character's state and animation based on movement
    if (moved) {
        // Don't change state if we're in PATTERN_EFFECT with HIDE
        if (!(obj_character[active_character].state == STATE_PATTERN_EFFECT && 
              player_pattern_effect_in_progress == PTRN_HIDE)) {
            obj_character[active_character].state = STATE_WALKING;
        }
    } else if (obj_character[active_character].state == STATE_WALKING) {
        obj_character[active_character].state = STATE_IDLE;
    }
    update_character_animation();
}

void handle_character_movement(s16 dx, s16 dy)    // Update character position with collision and screen boundary checks
{
    // dx Horizontal movement (-1 for left, 1 for right, 0 for no horizontal movement)
    // dy Vertical movement (-1 for up, 1 for down, 0 for no vertical movement)

    s16 current_x = obj_character[active_character].x;
    s16 new_x = current_x + dx;
    s16 new_y = obj_character[active_character].y + dy;
    u8 player_y_size = obj_character[active_character].y_size;
    bool direction_changed = false;

    // Check if we're changing horizontal direction
    if (dx != 0 && ((dx < 0 && !obj_character[active_character].flipH) ||
                    (dx > 0 && obj_character[active_character].flipH))) {
        direction_changed = true;
        num_colls = 0; // Reset collision counter

        // Check for collision at current position
        while ((detect_char_enemy_collision(active_character, current_x, new_y) != ENEMY_NONE ||
               detect_char_item_collision(active_character, current_x, new_y) != ITEM_NONE ||
               detect_char_char_collision(active_character, current_x, new_y) != CHR_NONE) &&
               num_colls < MAX_COLLISIONS) {
            
            // Move one pixel in new direction
            current_x += dx;
            num_colls++;

            // Stay within screen boundaries
            if (current_x < x_limit_min || current_x > x_limit_max) {
                break;
            }
        }
        new_x = current_x; // Update new_x with adjusted position
    }

    // If we haven't changed direction, check collision at new position
    if (!direction_changed &&
        ((detect_char_enemy_collision(active_character, new_x, new_y) != ENEMY_NONE) ||
         (detect_char_item_collision(active_character, new_x, new_y) != ITEM_NONE) ||
         (detect_char_char_collision(active_character, new_x, new_y) != CHR_NONE))) {
        return; // Collision detected, don't move
    }

    bool position_updated = false;

    // Handle horizontal movement
    if (dx != 0) {
        if ((background_scroll_mode == BG_SCRL_USER_RIGHT || background_scroll_mode == BG_SCRL_USER_LEFT) &&
            (new_x < x_limit_min || new_x > x_limit_max)) {
            // Player is trying to move outside screen boundaries, and scroll mode is user dependent
            scroll_background(dx); // Try to scroll
            position_updated = true;
        }
        else if (new_x >= x_limit_min && new_x <= x_limit_max) {
            // Update character position and flip state
            obj_character[active_character].x = new_x;
            obj_character[active_character].flipH = (dx < 0); // Flip character sprite if moving left
            position_updated = true;
        }
        // If new_x is outside boundaries and not in scrolling mode, do nothing
    }

    // Handle vertical movement
    if (dy != 0 && new_y + player_y_size >= y_limit_min && new_y + player_y_size <= y_limit_max) {
        obj_character[active_character].y = new_y;
        position_updated = true;
    }

    // Update character sprite position if any movement occurred
    if (position_updated) {
        update_character(active_character);
    }
}

void update_character_animation(void)    // Set appropriate animation based on character state
{
    // Don't change animation if we're in PATTERN_EFFECT with HIDE
    if (obj_character[active_character].state == STATE_PATTERN_EFFECT && 
        player_pattern_effect_in_progress == PTRN_HIDE) {
        return;
    }

    switch (obj_character[active_character].state) {
        case STATE_WALKING:
            if (obj_character[active_character].animation != ANIM_WALK) {
                obj_character[active_character].animation = ANIM_WALK;
                update_character(active_character);
            }
            break;
        case STATE_IDLE:
            if (obj_character[active_character].animation != ANIM_IDLE) {
                anim_character(active_character, ANIM_IDLE);
            }
            break;
        case STATE_PLAYING_NOTE:
            if (obj_character[active_character].animation != ANIM_ACTION) {
                anim_character(active_character, ANIM_ACTION);
            }
            break;
        case STATE_PATTERN_EFFECT:
            if (obj_character[active_character].animation != ANIM_MAGIC) {
                anim_character(active_character, ANIM_MAGIC);
            }
            break;
        case STATE_PATTERN_EFFECT_FINISH:
            if (obj_character[active_character].animation != ANIM_IDLE) {
                anim_character(active_character, ANIM_IDLE);
            }
            obj_character[active_character].state = STATE_IDLE;
            break;
        default:
            break;
    }
}

void handle_action_buttons(u16 joy_value)    // Process action buttons for item interaction and musical notes
{
    // Only allow item interaction and note playing in IDLE or WALKING states
    if (obj_character[active_character].state != STATE_IDLE && 
        obj_character[active_character].state != STATE_WALKING) {
        return;
    }

    if (movement_active) {
        if (joy_value & BUTTON_A) { // Detect if the player is interacting with an item
            u16 nitem=detect_nearby_item();
            if (nitem!=ITEM_NONE) {
                pending_item_interaction=nitem;
                return;
            }
        }
    }

    if (player_patterns_enabled) { // Detect if player is trying to play a note
        if (joy_value & BUTTON_A) play_note(NOTE_MI);
        if (joy_value & BUTTON_B) play_note(NOTE_FA);
        if (joy_value & BUTTON_C) play_note(NOTE_SOL);
        if (joy_value & BUTTON_X) play_note(NOTE_LA);
        if (joy_value & BUTTON_Y) play_note(NOTE_SI);
        if (joy_value & BUTTON_Z) play_note(NOTE_DO);
    }
}

void handle_pause_button(u16 joy_value)    // Handle START button press to show pause screen
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
