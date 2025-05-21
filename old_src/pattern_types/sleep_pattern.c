#include "sleep_pattern.h"
#include "../statemachine.h"
#include "../characters.h"
#include "../enemies.h"
#include "../interface.h"

// External variables needed
extern bool is_combat_active;
extern u16 player_pattern_effect_in_progress;
extern u16 player_pattern_effect_time;
extern bool player_pattern_effect_reversed;
extern u16 active_character;

bool sleep_pattern_can_use(void) {
    // Currently not usable - this is a placeholder for future implementation
    return false;
}

void sleep_pattern_launch(StateMachine* sm) {
    // Visual and sound effects
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_SLEEP, true, true);
    play_pattern_sound(PTRN_SLEEP);
    
    // Update state machine
    sm->pattern_system.effect_type = PTRN_SLEEP;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_SLEEP;
    player_pattern_effect_time = 1;
    player_pattern_effect_reversed = sm->is_reversed;
}

void sleep_pattern_do(StateMachine* sm) {
    // Do nothing by now, just wait for the effect to finish
    
    // Update duration
    sm->pattern_system.effect_duration++;
    
    // For compatibility with existing code
    player_pattern_effect_time++;
    
    // Brief visual effect
    if (sm->pattern_system.effect_duration >= 30) {
        show_pattern_icon(PTRN_SLEEP, false, false);
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void sleep_pattern_finish(StateMachine* sm) {
    // Clean up visual effects
    show_pattern_icon(PTRN_SLEEP, false, false);
    
    // Reset state
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_time = 0;
    player_pattern_effect_reversed = false;
    
    // Update character animation
    obj_character[active_character].state = STATE_IDLE;
    anim_character(active_character, ANIM_IDLE);
}
