#include "hide_pattern.h"
#include "../statemachine.h"
#include "../characters.h"
#include "../interface.h"

// External variables needed
extern u16 player_pattern_effect_in_progress;
extern u16 player_pattern_effect_time;
extern bool player_pattern_effect_reversed;
extern u16 active_character;
extern bool movement_active;

bool hide_pattern_can_use(void) {
    // Currently no restrictions on hide pattern
    return true;
}

void hide_pattern_launch(StateMachine* sm) {
    // Visual and sound effects
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_HIDE, true, true);
    play_pattern_sound(PTRN_HIDE);
    
    // Update state machine
    sm->pattern_system.effect_type = PTRN_HIDE;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
    
    // Allow movement while hidden
    movement_active = true;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_HIDE;
    player_pattern_effect_time = 1;
    player_pattern_effect_reversed = sm->is_reversed;
}

void hide_pattern_do(StateMachine* sm) {
    u16 max_effect_time = 400;
    
    // Create flickering effect
    if (sm->pattern_system.effect_duration % 2 == 0) {
        show_character(sm->entity_id, true);
    } else {
        show_character(sm->entity_id, false);
    }
    
    // Update duration
    sm->pattern_system.effect_duration++;
    
    // For compatibility with existing code
    player_pattern_effect_time++;
    
    // Check if effect is complete
    if (sm->pattern_system.effect_duration >= max_effect_time) {
        // Ensure character is visible
        show_character(sm->entity_id, true);
        show_pattern_icon(PTRN_HIDE, false, false);
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void hide_pattern_finish(StateMachine* sm) {
    // Ensure character is visible
    show_character(sm->entity_id, true);
    show_pattern_icon(PTRN_HIDE, false, false);
    
    // Reset state
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    player_pattern_effect_reversed = false;
    
    // Disable movement
    movement_active = false;
    
    // Update character animation
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
}
