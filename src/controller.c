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
        obj_character[active_character].state = STATE_WALKING;
    } else if (obj_character[active_character].state == STATE_WALKING) {
        obj_character[active_character].state = STATE_IDLE;
    }
    update_character_animations();
}

void handle_character_movement(s16 dx, s16 dy)    // Update character position with collision and screen boundary checks
{
    // dx Horizontal movement (-1 for left, 1 for right, 0 for no horizontal movement)
    // dy Vertical movement (-1 for up, 1 for down, 0 for no vertical movement)

    fastfix32 step = obj_character[active_character].speed;
    fastfix32 current_x_fixed = obj_character[active_character].x;
    fastfix32 current_y_fixed = obj_character[active_character].y;
    fastfix32 new_x_fixed = current_x_fixed;
    fastfix32 new_y_fixed = current_y_fixed;

    if (dx != 0) new_x_fixed += (dx > 0 ? step : -step);
    if (dy != 0) new_y_fixed += (dy > 0 ? step : -step);

    s16 current_x = FASTFIX32_TO_INT(current_x_fixed);
    s16 current_y = FASTFIX32_TO_INT(current_y_fixed);
    s16 new_x = FASTFIX32_TO_INT(new_x_fixed);
    s16 new_y = FASTFIX32_TO_INT(new_y_fixed);
    u8 player_y_size = obj_character[active_character].y_size;
    bool direction_changed = false;
    bool scroll_user_mode =
        (background_scroll_mode == BG_SCRL_USER_RIGHT ||
         background_scroll_mode == BG_SCRL_USER_LEFT);
    bool can_scroll = scroll_user_mode && player_scroll_active;
    bool use_x_limits = !scroll_user_mode || !player_scroll_active;

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
            if ((use_x_limits &&
                 (test_x < x_limit_min || test_x > x_limit_max)) ||
                test_y + player_y_size < y_limit_min ||
                test_y + player_y_size > y_limit_max)
            {
                break;
            }
        }

        // Update new position to where we found no collision
        new_x = test_x;
        new_y = test_y;
        new_x_fixed = FASTFIX32_FROM_INT(new_x);
        new_y_fixed = FASTFIX32_FROM_INT(new_y);
    }

    bool position_updated = false;

    // Handle horizontal movement
    if (dx != 0) {
        bool at_scroll_edge =
            (dx < 0 && new_x <= SCROLL_START_DISTANCE) ||
            (dx > 0 &&
             new_x + obj_character[active_character].x_size >=
                 SCREEN_WIDTH - SCROLL_START_DISTANCE);

        bool can_scroll_further =
            (dx < 0 && offset_BGA > 0) ||
            (dx > 0 && offset_BGA < (background_width - SCREEN_WIDTH));

        if (can_scroll && at_scroll_edge && can_scroll_further) {
            // Character reached screen edge → start scrolling
            scroll_background(dx);
            wait_for_followers(dx);
            new_x_fixed = current_x_fixed;
            position_updated = true;
        }
        else if (!use_x_limits ||
                 (new_x >= x_limit_min && new_x <= x_limit_max)) {
            // Update character position and flip state
            obj_character[active_character].x = new_x_fixed;
            if (direction_changed) {
                obj_character[active_character].flipH = (dx < 0);
            }
            position_updated = true;
        }
    }

    // Handle vertical movement
    if (dy != 0) {
        if (new_y + player_y_size >= y_limit_min &&
            new_y + player_y_size <= y_limit_max) {
            obj_character[active_character].y = new_y_fixed;
            position_updated = true;
        } else {
            new_y_fixed = current_y_fixed;
        }
    }

    // Update character sprite position if any movement occurred
    if (position_updated) {
        update_character(active_character);
    }
}

void handle_action_buttons(u16 joy_value)    // Process action buttons for item interaction and musical notes
{

    if (movement_active) {
        if (joy_value & BUTTON_A) { // Detect if the player is interacting with an item
            u16 nitem=detect_nearby_item();
            if (nitem!=ITEM_NONE) {
                dprintf(2,"  - Player pressed A, interacting with item %d", nitem);
                last_interacted_item=nitem;
                // Don't return here, continue to process note playing
            }
        }
    }

    // Process musical notes if player patterns are enabled and ther's not an active pattern launched
    if (player_patterns_enabled &&
        combat_state != COMBAT_STATE_PLAYER_EFFECT)
    {
        dprintf(3,"  - Checking buttons. Player pressed action button(s): joy_value=0x%04X", joy_value);
        if (joy_value & BUTTON_A) { pattern_player_add_note(NOTE_MI);  }
        if (joy_value & BUTTON_B) { pattern_player_add_note(NOTE_FA);  }
        if (joy_value & BUTTON_C) { pattern_player_add_note(NOTE_SOL); }
        if (joy_value & BUTTON_X) { pattern_player_add_note(NOTE_LA);  }
        if (joy_value & BUTTON_Y) { pattern_player_add_note(NOTE_SI);  }
        if (joy_value & BUTTON_Z) { pattern_player_add_note(NOTE_DO);  }
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

// Check follower distance when scrolling and wait if they lag behind
void wait_for_followers(s16 dx)
{
    for (u16 chr = 0; chr < MAX_CHR; chr++)
    {
        if (chr == active_character) continue;

        if (!obj_character[chr].active || !obj_character[chr].follows_character)
            continue;

        // Follower is in in the edge of the screen, so we need to wait for it (margin is MIN_FOLLOW_DISTANCE)
        // dx is the direction of the active character movement (-1 for left, 1 for right)
        if ((dx > 0 && FASTFIX32_TO_INT(obj_character[chr].x) < MAX_FOLLOW_DISTANCE) ||
            (dx < 0 && FASTFIX32_TO_INT(obj_character[chr].x) > (x_limit_max - MAX_FOLLOW_DISTANCE)))
        {
            dprintf(2,"  - Waiting for follower %d to catch up", chr);

            // Look back and wait
            look_left(active_character, (dx > 0));
            obj_character[active_character].state = STATE_IDLE;
            update_character(active_character);
            update_character_animations();

            bool old_movement = movement_active;
            movement_active = FALSE;
            for (u16 i=0; i < FOLLOW_WAIT_DISTANCE; i++)
            {
                next_frame(true);
            }
            movement_active = old_movement;
            look_left(active_character, (dx < 0)); // Look back to the right if moving right
        break; // Wait for a single follower only
        }
    }
}
