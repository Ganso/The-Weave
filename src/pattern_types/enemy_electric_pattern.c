#include "enemy_electric_pattern.h"
#include "../statemachine.h"
#include "../counter_spell.h"
#include "../characters.h"
#include "../enemies.h"
#include "../combat.h"
#include "../dialogs.h"
#include "../interface.h"

// External variables needed
extern u16 player_pattern_effect_in_progress;
extern bool player_pattern_effect_reversed;
extern u16 active_character;
extern u16 enemy_attack_pattern;
extern u16 enemy_attack_effect_time;
extern bool enemy_attack_effect_in_progress;

void enemy_electric_pattern_launch(StateMachine* sm) {
    // Visual effect - enemy animation and sound
    u16 enemy_id = sm->entity_id - ENEMY_ENTITY_ID_BASE;
    anim_enemy(enemy_id, ANIM_MAGIC);
    play_pattern_sound(PTRN_ELECTRIC);
}

void enemy_electric_pattern_do(StateMachine* sm) {
    // Only log every 60 frames to reduce spam
    if (sm->pattern_system.effect_duration % 60 == 0) {
        kprintf("enemy_electric_pattern_do: effect_duration=%d", sm->pattern_system.effect_duration);
    }
    
    // Visual thunder effect
    VDP_setHilightShadow((sm->pattern_system.effect_duration % 2) == 0);
    SPR_update();
    
    // Increment the effect duration
    sm->pattern_system.effect_duration++;

    // If counter-spell already succeeded, don't do anything
    if (counter_spell_success) {
        return;
    }
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    // Create lightning flash effect
    kprintf("Creating lightning flash effect");
    if (sm->pattern_system.effect_duration % 2 == 0) {
        VDP_setHilightShadow(true);
    } else {
        VDP_setHilightShadow(false);
    }
    SPR_update();
    
    // Check for player counter-spell
    if (player_pattern_effect_in_progress == PTRN_ELECTRIC && player_pattern_effect_reversed == true) {
        // Set the counter-spell success flag
        counter_spell_success = true;
        
        // Stop all visual effects immediately
        VDP_setHilightShadow(false);
        
        // Show success message
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar|al revés" - (EN) "I should maybe|think backwards"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // Damage the enemy
        hit_enemy(sm->entity_id - ENEMY_ENTITY_ID_BASE);
        
        // Clean up all active notes
        cleanup_enemy_notes();
        
        // Reset enemy state
        u16 enemy_id = sm->entity_id - ENEMY_ENTITY_ID_BASE;
        anim_enemy(enemy_id, ANIM_IDLE);
        enemy_attack_effect_in_progress = false;
        obj_enemy[enemy_id].obj_character.state = STATE_IDLE;
        
        // Set the cooldown to 75% of the recharge time to allow the enemy to attack again after a delay
        u16 current_pattern = enemy_attack_pattern;
        obj_enemy[enemy_id].last_pattern_time[current_pattern] = obj_Pattern_Enemy[current_pattern].recharge_time * 3 / 4;
        
        // Reset attack state
        enemy_attack_pattern = PTRN_EN_NONE;
        
        // Reset player state
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_reversed = false;
        obj_character[active_character].state = STATE_IDLE;
        anim_character(active_character, ANIM_IDLE);
        
        // Hide combat interface
        show_or_hide_enemy_combat_interface(false);
        
        // Update sprites to reflect changes
        SPR_update();
        
        return;
    }
    
    // If player is playing notes, extend the effect time to give them a chance to counter
    if (player_is_playing_notes && sm->pattern_system.effect_duration >= calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30) {
        // Keep the effect going a bit longer
        sm->pattern_system.effect_duration = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
    }
    
    // Update duration
    sm->pattern_system.effect_duration++;
}

void enemy_electric_pattern_finish(StateMachine* sm) {
    kprintf("enemy_electric_pattern_finish called for enemy %d", sm->entity_id - ENEMY_ENTITY_ID_BASE);
    
    // If counter-spell already succeeded, don't do anything
    if (counter_spell_success) {
        VDP_setHilightShadow(false);
        kprintf("Counter spell succeeded, skipping damage");
        return;
    }
    
    VDP_setHilightShadow(false);
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    if (player_is_playing_notes) {
        // Player is still trying to counter, give them more time
        kprintf("Player is still trying to counter, extending effect time");
        
        // Extend the effect time to give them a chance to counter
        sm->pattern_system.effect_duration = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
        
        // Don't apply damage yet
        return;
    }
    
    // Player failed to counter - apply damage
    kprintf("Player failed to counter, applying damage");
    hit_caracter(active_character);
    show_or_hide_interface(false);
    show_or_hide_enemy_combat_interface(false);
    talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
    
    // Only show the hint if they haven't successfully countered before
    if (!counter_spell_success) {
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar|al revés" - (EN) "I should maybe|think backwards"
    }
    
    show_or_hide_interface(true);
    show_or_hide_enemy_combat_interface(true);
    
    // Reset enemy state
    u16 enemy_id = sm->entity_id - ENEMY_ENTITY_ID_BASE;
    anim_enemy(enemy_id, ANIM_IDLE);
    
    // Reset all effect flags
    enemy_attack_effect_in_progress = false;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_duration = 0;
    
    // Reset enemy state
    obj_enemy[enemy_id].obj_character.state = STATE_IDLE;
    sm->current_state = SM_STATE_IDLE;
    
    // Reset attack state
    if (enemy_attacking == enemy_id) {
        kprintf("Resetting enemy_attacking from %d to ENEMY_NONE", enemy_attacking);
        enemy_attacking = 254; // ENEMY_NONE value (254)
    }
}
