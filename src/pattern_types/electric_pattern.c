#include "electric_pattern.h"
#include "../statemachine.h"
#include "../counter_spell.h"
#include "../combat.h"
#include "../dialogs.h"
#include "../interface.h"
#include "../characters.h"
#include "../enemies.h"

// External variables needed
extern bool is_combat_active;
extern u16 enemy_attacking;
extern u16 enemy_attack_pattern;
extern bool enemy_attack_effect_in_progress;
extern u16 player_pattern_effect_in_progress;
extern u16 player_pattern_effect_time;
extern bool player_pattern_effect_reversed;
extern u16 active_character;

bool electric_pattern_can_use(void) {
    // Check if we're in combat with an enemy using electric attack
    if (is_combat_active && enemy_attacking != ENEMY_NONE &&
        enemy_attack_effect_in_progress && enemy_attack_pattern == PTRN_EN_ELECTIC) {
        return true;
    }
    
    // Check if player is hidden
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        // Can't use thunder while hidden
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][9]); // (ES) "No puedo lanzar hechizos|si estoy escondido" - (EN) "I can't launch spells|while hiding"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        return false;
    }
    
    return true;
}

void electric_pattern_launch(StateMachine* sm) {
    // Visual and sound effects
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_ELECTRIC, true, true);
    play_pattern_sound(PTRN_ELECTRIC);
    
    // Update state machine
    sm->pattern_system.effect_type = PTRN_ELECTRIC;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
    
    // For compatibility with existing code
    player_pattern_effect_in_progress = PTRN_ELECTRIC;
    player_pattern_effect_time = 0;
    player_pattern_effect_reversed = sm->is_reversed;
}

void electric_pattern_do(StateMachine* sm) {
    // If a counter-spell has succeeded, finish immediately
    if (counter_spell_success && sm->is_reversed) {
        kprintf("Counter spell succeeded, finishing electric pattern effect");
        
        // Reset state machine
        sm->current_state = SM_STATE_IDLE;
        sm->timer = 0;
        sm->effect_time = 0;
        sm->pattern_system.effect_in_progress = false;
        sm->pattern_system.effect_type = PTRN_NONE;
        sm->pattern_system.effect_duration = 0;
        
        // Reset visual effects
        VDP_setHilightShadow(false);
        show_pattern_icon(PTRN_ELECTRIC, false, false);
        
        // Reset global state
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_time = 0;
        player_pattern_effect_reversed = false;
        
        // Update character animation
        obj_character[active_character].state = STATE_IDLE;
        anim_character(active_character, ANIM_IDLE);
        
        return;
    }
    
    // Visual thunder effect
    VDP_setHilightShadow((sm->pattern_system.effect_duration % 2) == 0);
    SPR_update();
    SYS_doVBlankProcess();
    
    // Apply combat effects
    if (is_combat_active && sm->pattern_system.effect_duration == 10) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].class_id == ENEMY_CLS_3HEADMONKEY &&
                obj_enemy[nenemy].hitpoints > 0) {
                hit_enemy(nenemy);
                if (enemy_attack_pattern == PTRN_EN_BITE) {
                    enemy_attack_pattern = PTRN_EN_NONE;
                    finish_enemy_pattern_effect();
                }
            }
        }
    }
    
    // Update duration
    sm->pattern_system.effect_duration++;
    
    // For compatibility with existing code
    player_pattern_effect_time++;
    
    // Check if effect is complete
    if (sm->pattern_system.effect_duration >= 20) {
        // Ensure visual effect is disabled
        VDP_setHilightShadow(false);
        show_pattern_icon(PTRN_ELECTRIC, false, false);
        SPR_update();
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void electric_pattern_finish(StateMachine* sm) {
    // Ensure visual effect is disabled
    VDP_setHilightShadow(false);
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    SPR_update();
    
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
    update_character_animation();
}
