#include "bite_pattern.h"
#include "../statemachine.h"
#include "../characters.h"
#include "../enemies.h"
#include "../combat.h"
#include "../dialogs.h"
#include "../interface.h"

// External variables needed
extern u16 player_pattern_effect_in_progress;
extern u16 active_character;

void bite_pattern_launch(StateMachine* sm) {
    // Visual effect - enemy animation
    u16 enemy_id = sm->entity_id - ENEMY_ENTITY_ID_BASE;
    anim_enemy(enemy_id, ANIM_MAGIC);
}

void bite_pattern_do(StateMachine* sm) {
    // If player is hidden, extend the effect time to avoid damage
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        // Keep the effect going but don't apply damage
        if (sm->pattern_system.effect_duration >= calc_ticks(MAX_EFFECT_TIME_BITE) - 1) {
            sm->pattern_system.effect_duration = calc_ticks(MAX_EFFECT_TIME_BITE) - 1;
        }
    }
    
    // Update duration
    sm->pattern_system.effect_duration++;
}

void bite_pattern_finish(StateMachine* sm) {
    kprintf("bite_pattern_finish called for enemy %d", sm->entity_id - ENEMY_ENTITY_ID_BASE);
    
    // If player is hidden, skip damage
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        kprintf("Player is hidden, skipping damage");
        return;
    }

    // Player failed to hide - apply damage
    kprintf("Player failed to hide, applying damage");
    show_character(active_character, true);
    hit_caracter(active_character);
    
    // Show dialog
    show_or_hide_interface(false);
    show_or_hide_enemy_combat_interface(false);
    talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
    talk_dialog(&dialogs[ACT1_DIALOG3][4]); // (ES) "Puedo probar a esconderme|o tratar de invocar|al trueno" - (EN) "I could try to hide|or attempt to summon|the thunder"
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
