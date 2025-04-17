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

    // Calculate the new position
    s16 new_x = obj_character[active_character].x + dx;
    s16 new_y = obj_character[active_character].y + dy;
    u8 player_y_size = obj_character[active_character].y_size;
    
    // ULTRA SIMPLE SOLUTION
    // Static counter for consecutive collisions
    static u8 collision_counter = 0;
    static const u8 MAX_COLLISIONS = 20;
    
    // Track direction changes
    static s16 last_dx = 0;
    static s16 last_dy = 0;
    bool direction_changed = (dx != last_dx || dy != last_dy);
    
    // Store current direction for next frame
    last_dx = dx;
    last_dy = dy;
    
    // Initialize collision flags
    last_collision_type = COLLISION_NONE;
    bool can_move = true;
    u8 collision_response = RESPONSE_BLOCK;
    
    // Debug movement attempt
    kprintf("ULTRA SIMPLE: dx=%d, dy=%d, changed=%d, counter=%d",
            dx, dy, direction_changed, collision_counter);

    // Only check for collisions if we're actually trying to move
    if (dx != 0 || dy != 0) {
        // First, check if there's a collision at the new position
        u16 enemy_collision = detect_char_enemy_collision(active_character, new_x, new_y);
        u16 item_collision = detect_char_item_collision(active_character, new_x, new_y);
        u16 char_collision = detect_char_char_collision(active_character, new_x, new_y);
        
        // If there's any collision at the new position
        if (enemy_collision != ENEMY_NONE || item_collision != ITEM_NONE || char_collision != CHR_NONE) {
            // Debug which collision was detected (only when a collision occurs)
            if (enemy_collision != ENEMY_NONE) kprintf("Collision with enemy %d detected", enemy_collision);
            if (item_collision != ITEM_NONE) kprintf("Collision with item %d detected", item_collision);
            if (char_collision != CHR_NONE) kprintf("Collision with character %d detected", char_collision);
            
            can_move = false;
            last_collision_type = COLLISION_DIAGONAL;
            
            // Determine collision response based on movement type
            if (dx != 0 && dy != 0) {
                // For diagonal movement, try sliding
                collision_response = RESPONSE_SLIDE;
            } else {
                // For cardinal movement, just block
                collision_response = RESPONSE_BLOCK;
            }
            
            // Handle collision based on response type
            if (collision_response == RESPONSE_SLIDE) {
                // Try horizontal movement only
                s16 horiz_x = obj_character[active_character].x + dx;
                s16 horiz_y = obj_character[active_character].y;
                
                bool horiz_collision = false;
                // Check for collision with horizontal movement only
                if (detect_char_enemy_collision(active_character, horiz_x, horiz_y) != ENEMY_NONE ||
                    detect_char_item_collision(active_character, horiz_x, horiz_y) != ITEM_NONE ||
                    detect_char_char_collision(active_character, horiz_x, horiz_y) != CHR_NONE) {
                    horiz_collision = true;
                    last_collision_type = COLLISION_HORIZONTAL;
                }
                
                // Try vertical movement only
                s16 vert_x = obj_character[active_character].x;
                s16 vert_y = obj_character[active_character].y + dy;
                
                bool vert_collision = false;
                // Check for collision with vertical movement only
                if (detect_char_enemy_collision(active_character, vert_x, vert_y) != ENEMY_NONE ||
                    detect_char_item_collision(active_character, vert_x, vert_y) != ITEM_NONE ||
                    detect_char_char_collision(active_character, vert_x, vert_y) != CHR_NONE) {
                    vert_collision = true;
                    last_collision_type = COLLISION_VERTICAL;
                }
                
                // Implement sliding
                if (!horiz_collision) {
                    kprintf("Sliding horizontally");
                    new_y = obj_character[active_character].y; // Keep y the same, only move in x
                    can_move = true;
                }
                else if (!vert_collision) {
                    kprintf("Sliding vertically");
                    new_x = obj_character[active_character].x; // Keep x the same, only move in y
                    can_move = true;
                }
                else {
                    kprintf("Cannot slide in any direction");
                    
                    // ULTRA SIMPLE SOLUTION
                    // Increment collision counter
                    collision_counter++;
                    kprintf("ULTRA SIMPLE: Collision detected! Counter: %d/%d",
                            collision_counter, MAX_COLLISIONS);
                    
                    // If direction changed or counter reached max, allow movement
                    if (direction_changed) {
                        kprintf("ULTRA SIMPLE: Direction changed, allowing movement");
                        collision_counter = 0;
                        can_move = true;
                    } else if (collision_counter >= MAX_COLLISIONS) {
                        kprintf("ULTRA SIMPLE: Max collisions reached, allowing movement");
                        collision_counter = 0;
                        can_move = true;
                    }
                }
            }
        }
    }

    // If we can't move after all checks, return
    if (!can_move) {
        return;
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
            // In any other circumstance, if new x is between x_limit_min and x_limit_max, update x in object
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
