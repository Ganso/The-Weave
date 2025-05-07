#include "open_pattern.h"
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

bool open_pattern_can_use(void) {
    // Currently not usable - this is a placeholder for future implementation
    return false;
}

void open_pattern_launch(StateMachine* sm) {
    // Visual and sound effects
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_OPEN, true, true);
    play_pattern_sound(PTRN_OPEN);
    
    // Update state machine
    sm->pattern_system.effect_type = PTRN_OPEN;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_OPEN;
    player_pattern_effect_time = 1;
    player_pattern_effect_reversed = sm->is_reversed;
}

void open_pattern_do(StateMachine* sm) {
    // In combat, make enemies more vulnerable
    if (is_combat_active && sm->pattern_system.effect_duration == 10) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].hitpoints > 0) {
                // Make enemy more vulnerable (receives more damage)
                obj_enemy[nenemy].vulnerable = true;
                obj_enemy[nenemy].vulnerable_time = 300; // Duration of vulnerability
            }
        }
    }
    
    // Update duration
    sm->pattern_system.effect_duration++;
    
    // For compatibility with existing code
    player_pattern_effect_time++;
    
    // Brief visual effect
    if (sm->pattern_system.effect_duration >= 30) {
        show_pattern_icon(PTRN_OPEN, false, false);
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void open_pattern_finish(StateMachine* sm) {
    // Clean up visual effects
    show_pattern_icon(PTRN_OPEN, false, false);
    
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
