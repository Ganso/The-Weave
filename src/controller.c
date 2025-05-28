#include "globals.h"

void joy_check(void)    // Process joystick input each frame for movement, actions, and pause
{
    static u16 frame_counter = 0;
    frame_counter++;
    
    // Read the current state of the joystick
    u16 joy_value = JOY_readJoypad(JOY_ALL);

    // Only process movement and action buttons if movement is allowed
    if (movement_active) {
        if (obj_character[active_character].state == STATE_IDLE ||
            obj_character[active_character].state == STATE_WALKING ||
        (obj_character[active_character].state == STATE_PATTERN_EFFECT &&
            combatContext.activePattern == PATTERN_HIDE))
        {
            dprintf(3,"Handling movement");
            handle_movement(joy_value);
        }
        
        // Always check action buttons, even during pattern effects
        dprintf(3,"Handling action buttons");
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
        if (!(obj_character[active_character].state == STATE_PATTERN_EFFECT && combatContext.activePattern == PATTERN_HIDE)) {
            obj_character[active_character].state = STATE_WALKING;
        }
    } else if (obj_character[active_character].state == STATE_WALKING) {
        obj_character[active_character].state = STATE_IDLE;
    }
    update_character_animations();
}

void handle_character_movement(s16 dx, s16 dy)    // Update character position with collision and screen boundary checks
{
    // dx Horizontal movement (-1 for left, 1 for right, 0 for no horizontal movement)
    // dy Vertical movement (-1 for up, 1 for down, 0 for no vertical movement)

    s16 current_x = obj_character[active_character].x;
    s16 current_y = obj_character[active_character].y;
    s16 new_x = current_x + dx;
    s16 new_y = current_y + dy;
    u8 player_y_size = obj_character[active_character].y_size;
    bool direction_changed = false;

    // Check if we're changing horizontal direction
    if (dx != 0) {
        direction_changed = ((dx < 0 && !obj_character[active_character].flipH) ||
                           (dx > 0 && obj_character[active_character].flipH));
    }

    // Check for collision at new position
    if ((detect_char_enemy_collision(active_character, new_x, new_y) != ENEMY_NONE) ||
        (detect_char_item_collision(active_character, new_x, new_y) != ITEM_NONE) ||
        (detect_char_char_collision(active_character, new_x, new_y) != CHR_NONE)) {
        
        num_colls = 0; // Reset collision counter
        
        // If changing direction, try moving in new direction
        // If not changing direction, try moving in opposite direction
        s16 move_dx = direction_changed ? dx : -dx;
        s16 move_dy = direction_changed ? dy : -dy;
        s16 test_x = direction_changed ? current_x : new_x;
        s16 test_y = direction_changed ? current_y : new_y;

        // Move pixel by pixel until no collision or MAX_COLLISIONS reached
        while ((detect_char_enemy_collision(active_character, test_x, test_y) != ENEMY_NONE ||
               detect_char_item_collision(active_character, test_x, test_y) != ITEM_NONE ||
               detect_char_char_collision(active_character, test_x, test_y) != CHR_NONE) &&
               num_colls < MAX_COLLISIONS) {
            
            test_x += move_dx;
            test_y += move_dy;
            num_colls++;

            // Stay within screen boundaries
            if (test_x < x_limit_min || test_x > x_limit_max ||
                test_y + player_y_size < y_limit_min || test_y + player_y_size > y_limit_max) {
                break;
            }
        }

        // Update new position to where we found no collision
        new_x = test_x;
        new_y = test_y;
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
            if (direction_changed) {
                obj_character[active_character].flipH = (dx < 0); // Only update flip if direction changed
            }
            position_updated = true;
        }
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

void handle_action_buttons(u16 joy_value)    // Process action buttons for item interaction and musical notes
{
    // // Only allow item interaction and note playing in IDLE, WALKING or PLAYING_NOTE states
    // if (obj_character[active_character].state != STATE_IDLE &&
    //     obj_character[active_character].state != STATE_WALKING &&
    //     obj_character[active_character].state != STATE_PLAYING_NOTE) {
    //         dprintf(2,"  - Skipping action buttons: character state is %d", obj_character[active_character].state);
    //     return;
    // }

    if (movement_active) {
        if (joy_value & BUTTON_A) { // Detect if the player is interacting with an item
            u16 nitem=detect_nearby_item();
            if (nitem!=ITEM_NONE) {
                dprintf(2,"  - Player pressed A, interacting with item %d", nitem);
                TODO_item_interaction=nitem;
                // Don't return here, continue to process note playing
            }
        }
    }

    // Process musical notes if player patterns are enabled and ther's not an active pattern launched
    if (player_patterns_enabled &&
        combat_state != COMBAT_STATE_PLAYER_PLAYING &&
        combat_state != COMBAT_STATE_PLAYER_EFFECT)
    {
        dprintf(3,"  - Checking buttons. Player pressed action button(s): joy_value=0x%04X", joy_value);
        if (joy_value & BUTTON_A) { patternPlayerAddNote(NOTE_MI);  }
        if (joy_value & BUTTON_B) { patternPlayerAddNote(NOTE_FA);  }
        if (joy_value & BUTTON_C) { patternPlayerAddNote(NOTE_SOL); }
        if (joy_value & BUTTON_X) { patternPlayerAddNote(NOTE_LA);  }
        if (joy_value & BUTTON_Y) { patternPlayerAddNote(NOTE_SI);  }
        if (joy_value & BUTTON_Z) { patternPlayerAddNote(NOTE_DO);  }
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
