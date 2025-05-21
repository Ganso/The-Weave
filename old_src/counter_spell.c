#include "counter_spell.h"
#include "statemachine.h"
#include "patterns_registry.h"
#include "combat.h"
#include "dialogs.h"
#include "interface.h"

// Global flag for counter-spell success - already defined in enemies_patterns.c
// bool counter_spell_success = false;
// u16 pending_counter_hit_enemy = ENEMY_NONE;

bool can_counter_spell(u16 player_pattern, u16 enemy_pattern, bool is_reverse) {
    // Currently only electric pattern can counter electric enemy pattern when reversed
    if (player_pattern == PTRN_ELECTRIC && 
        enemy_pattern == PTRN_EN_ELECTIC && 
        is_reverse) {
        return true;
    }
    return false;
}

// Helper function to completely reset all state after a counter-spell
void reset_all_states_after_counter_spell(u16 enemy_id) {
    kprintf("Performing complete state reset after counter-spell");
    
    // Reset global flags
    counter_spell_success = false;
    enemy_attack_effect_in_progress = false;
    enemy_attack_pattern = PTRN_EN_NONE;
    enemy_attack_pattern_notes = 0;
    enemy_attack_time = 0;
    enemy_attack_effect_time = 0;
    enemy_attacking = ENEMY_NONE;
    
    // Reset player pattern effect flags
    player_pattern_effect_in_progress = PTRN_NONE;
    player_pattern_effect_reversed = false;
    player_pattern_effect_time = 0;
    
    // Reset player state
    obj_character[active_character].state = STATE_IDLE;
    
    // Reset player state machine - use the global character_state_machines array
    player_state_machine.current_state = SM_STATE_IDLE;
    player_state_machine.pattern_system.effect_in_progress = false;
    player_state_machine.pattern_system.effect_type = PTRN_NONE;
    player_state_machine.pattern_system.effect_duration = 0;
    player_state_machine.pattern_system.effect_reversed = false;
    
    // Reset enemy state
    if (enemy_id != ENEMY_NONE && enemy_id < MAX_ENEMIES) {
        obj_enemy[enemy_id].obj_character.state = STATE_IDLE;
        enemy_state_machines[enemy_id].current_state = SM_STATE_IDLE;
        enemy_state_machines[enemy_id].pattern_system.effect_in_progress = false;
        enemy_state_machines[enemy_id].pattern_system.effect_type = PTRN_NONE;
        enemy_state_machines[enemy_id].pattern_system.effect_duration = 0;
        
        // Reset animations
        anim_enemy(enemy_id, ANIM_IDLE);
    }
    
    // Reset all other enemies too
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        if (i != enemy_id && obj_enemy[i].obj_character.active) {
            obj_enemy[i].obj_character.state = STATE_IDLE;
            enemy_state_machines[i].current_state = SM_STATE_IDLE;
        }
    }
    
    // Reset visual effects
    VDP_setHilightShadow(false);
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    
    // Update sprites
    SPR_update();
}

void execute_counter_spell(StateMachine* player_sm, StateMachine* enemy_sm, u16 pattern_id) {
    // Set counter-spell success flag
    counter_spell_success = true;
    
    // Update player state machine
    player_sm->current_state = SM_STATE_PATTERN_EFFECT;
    player_sm->active_pattern = pattern_id;
    player_sm->is_reversed = true;
    player_sm->pattern_system.effect_reversed = true;
    
    // Set global player pattern effect flags
    player_pattern_effect_in_progress = pattern_id;
    player_pattern_effect_reversed = true;
    
    // Configure callbacks
    Pattern* pattern = get_pattern(pattern_id, OWNER_PLAYER);
    if (pattern) {
        player_sm->launch_effect = pattern->launch;
        player_sm->do_effect = pattern->do_effect;
        player_sm->finish_effect = pattern->finish;
        
        // Launch the effect
        if (player_sm->launch_effect) {
            player_sm->launch_effect(player_sm);
        }
    }
    
    // Reset all enemy pattern cooldowns to a reasonable value
    // This ensures enemies will attack again soon after a counter-spell
    u16 enemy_id = enemy_sm->entity_id - ENEMY_ENTITY_ID_BASE;
    for (u8 i = 0; i < MAX_PATTERN_ENEMY; i++) {
        if (obj_enemy[enemy_id].obj_character.active) {
            // Set cooldown to 75% of max to allow for a reasonable delay
            obj_enemy[enemy_id].last_pattern_time[i] = obj_Pattern_Enemy[i].recharge_time * 3 / 4;
            kprintf("Reset pattern %d cooldown for enemy %d to %d",
                    i, enemy_id, obj_enemy[enemy_id].last_pattern_time[i]);
        }
    }
    
    // Also reset cooldowns for other enemies
    for (u8 other_enemy = 0; other_enemy < MAX_ENEMIES; other_enemy++) {
        if (other_enemy != enemy_id && obj_enemy[other_enemy].obj_character.active) {
            for (u8 i = 0; i < MAX_PATTERN_ENEMY; i++) {
                // Set cooldown to 50% of max for other enemies to allow them to attack sooner
                obj_enemy[other_enemy].last_pattern_time[i] = obj_Pattern_Enemy[i].recharge_time / 2;
            }
        }
    }
    
    // Hit the enemy - entity_id already includes ENEMY_ENTITY_ID_BASE
    kprintf("Counter spell hitting enemy %d (entity_id: %d)", enemy_id, enemy_sm->entity_id);
    hit_enemy(enemy_id);
    
    // Reset enemy attack state
    enemy_sm->current_state = SM_STATE_IDLE;
    enemy_sm->pattern_system.effect_in_progress = false;
    
    // Visual cleanup
    VDP_setHilightShadow(false);
}

void handle_counter_spell_result(bool success, u16 enemy_id, u16 pattern_id) {
    if (success) {
        // Show success message
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar|al revés" - (EN) "I should maybe|think backwards"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // Perform a complete reset of all states
        reset_all_states_after_counter_spell(enemy_id);
    } else {
        // Reset counter-spell flag
        counter_spell_success = false;
    }
}
