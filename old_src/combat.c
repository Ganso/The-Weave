#include "globals.h"
#include "patterns_registry.h"
#include "counter_spell.h"

// Referencia a la máquina de estados del jugador
extern StateMachine player_state_machine;
extern StateMachine enemy_state_machines[MAX_ENEMIES];

bool is_combat_active;    // Whether combat sequence is currently active

/**
 * Combat System
 * 
 * This module handles the core combat mechanics including:
 * - Combat initialization and cleanup
 * - Damage calculation and application
 * - Combat UI management (life counters, enemy faces, note indicators)
 * - State tracking for active combats
 */

void start_combat(bool start)    // Initialize or cleanup combat sequence with UI and enemy states
{
    u8 numenemy, npattern, nnote;

    if (start) {
        // Combat initialization
        setRandomSeed(frame_counter);
        is_combat_active = true;
        player_scroll_active = false;
        
        // Enviar mensaje a la máquina de estados del jugador
        StateMachine_SendMessage(&player_state_machine, MSG_COMBAT_START, 0);

        // Reset enemies to combat-ready state
        for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active) {
                // Reset enemy HP to max
                obj_enemy[numenemy].hitpoints = obj_enemy[numenemy].class.max_hitpoints;

                // Randomize pattern cooldowns to prevent synchronized attacks
                for (npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern]) {
                        u16 max_initial_cooldown = obj_Pattern_Enemy[npattern].recharge_time / 2;
                        obj_enemy[numenemy].last_pattern_time[npattern] = random() % max_initial_cooldown;
                    }
                }
            }
            
            // Inicializar la máquina de estados del enemigo
            StateMachine_Init(&enemy_state_machines[numenemy], ENEMY_ENTITY_ID_BASE + numenemy);
            enemy_state_machines[numenemy].owner_type = OWNER_ENEMY;
        }

        // Initialize combat state
        enemy_attacking = ENEMY_NONE;
        enemy_attack_pattern_notes = 0;
        enemy_attack_time = 0;
        enemy_attack_effect_in_progress = false;
        for (nnote = 0; nnote < 6; nnote++) {
            enemy_note_active[nnote] = false;
        }

        // Reset counter-spell state
        counter_spell_success = false;
        pending_counter_hit_enemy = ENEMY_NONE;

        // Setup combat UI
        spr_int_life_counter = SPR_addSprite(&int_life_counter_sprite, 164, 180, TILE_ATTR(PAL2, false, false, false));
        SPR_setVisibility(spr_int_life_counter, HIDDEN);
    }
    else {
        // Combat cleanup
        is_combat_active = false;
        player_scroll_active = true;
        
        // Enviar mensaje a la máquina de estados del jugador
        StateMachine_SendMessage(&player_state_machine, MSG_COMBAT_END, 0);
        
        // Reset all enemy state machines
        for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active) {
                StateMachine_SendMessage(&enemy_state_machines[numenemy], MSG_COMBAT_END, 0);
            }
        }
        
        // Cleanup life counter sprite
        if (spr_int_life_counter != NULL) {
            SPR_releaseSprite(spr_int_life_counter);
            spr_int_life_counter = NULL;
        }
    }
}

void hit_enemy(u16 nenemy)    // Apply damage to enemy, handle defeat, and update UI
{
    u16 remaining_enemies = 0;

    // Play hit sound effect
    play_sample(snd_player_hit_enemy,sizeof(snd_player_hit_enemy));

    // If enemy was attacking, clean up combat state
    if (enemy_attacking == nenemy) {
        // Clean up all active notes
        cleanup_enemy_notes();
        
        // Reset enemy state
        anim_enemy(nenemy, ANIM_IDLE);
        enemy_attack_effect_in_progress = false;
        obj_enemy[nenemy].obj_character.state = STATE_IDLE;
        if (enemy_attack_pattern < MAX_PATTERN_ENEMY) {
            obj_enemy[nenemy].last_pattern_time[enemy_attack_pattern] = 0;
        }
        enemy_attacking = ENEMY_NONE;
        
        // Hide combat interface
        show_or_hide_enemy_combat_interface(false);
    }

    // Apply damage and check for defeat
    obj_enemy[nenemy].hitpoints--;
    
    // Enviar mensaje a la máquina de estados del enemigo
    StateMachine_SendMessage(&enemy_state_machines[nenemy], MSG_ENEMY_HIT, 0);
    
    if (obj_enemy[nenemy].hitpoints == 0) {
        // Enemy defeated
        SPR_setVisibility(spr_int_life_counter, HIDDEN);
        
        // Enviar mensaje de enemigo derrotado
        StateMachine_SendMessage(&enemy_state_machines[nenemy], MSG_ENEMY_DEFEATED, 0);
        
        release_enemy(nenemy);

        // Check if all enemies defeated
        for (u16 i = 0; i < MAX_ENEMIES; i++) {
            if (obj_enemy[i].obj_character.active) {
                remaining_enemies++;
            }
        }
        
        if (remaining_enemies == 0) {
            start_combat(false);  // End combat when all enemies defeated
        }
    }
    else {
        // Enemy survived - Update UI
        if (spr_int_life_counter != NULL) {
            // Show damage animation
            if (nenemy != enemy_attacking && spr_enemy_face[nenemy] != NULL) {
                SPR_setVisibility(spr_enemy_face[nenemy], VISIBLE);
            }

            // Flash life counter to indicate damage
            SPR_setVisibility(spr_int_life_counter, VISIBLE);
            for (u8 frame = 0; frame < SCREEN_FPS; frame++) {
                // Alternate between current and previous HP for flash effect
                SPR_setAnim(spr_int_life_counter, obj_enemy[nenemy].hitpoints);
                SPR_update();
                SYS_doVBlankProcess();
                SPR_setAnim(spr_int_life_counter, obj_enemy[nenemy].hitpoints - 1);
                SPR_update();
                SYS_doVBlankProcess();
            }

            // Reset UI visibility
            if (nenemy != enemy_attacking && spr_enemy_face[nenemy] != NULL) {
                SPR_setVisibility(spr_enemy_face[nenemy], HIDDEN);
                SPR_setVisibility(spr_int_life_counter, HIDDEN);
            }
        }
    }
}

void hit_caracter(u16 nchar)    // Handle player character damage (currently just sound)
{
    play_sample(snd_player_hurt,sizeof(snd_player_hurt));
    
    // Enviar mensaje a la máquina de estados del jugador
    StateMachine_SendMessage(&player_state_machine, MSG_PLAYER_HIT, 0);
}

void show_or_hide_enemy_combat_interface(bool show)    // Toggle combat UI elements (faces, life counter, notes)
{
    if (show && interface_active && is_combat_active && enemy_attacking != ENEMY_NONE) {
        // Show attacking enemy's interface
        if (spr_enemy_face[enemy_attacking] != NULL) {
            SPR_setVisibility(spr_enemy_face[enemy_attacking], VISIBLE);
        }

        // Hide other enemy faces
        for (u16 i = 0; i < MAX_ENEMIES; i++) {
            if (i != enemy_attacking && spr_enemy_face[i] != NULL) {
                SPR_setVisibility(spr_enemy_face[i], HIDDEN);
            }
        }

        // Update life counter
        if (spr_int_life_counter != NULL) {
            SPR_setVisibility(spr_int_life_counter, VISIBLE);
            SPR_setAnim(spr_int_life_counter, obj_enemy[enemy_attacking].hitpoints - 1);
        }

        // Update note indicators
        for (u8 note = 0; note < 6; note++) {
            show_enemy_note(note + 1, enemy_note_active[note], false);
        }
    }
    else {
        // Hide all interface elements
        for (u16 i = 0; i < MAX_ENEMIES; i++) {
            if (spr_enemy_face[i] != NULL) {
                SPR_setVisibility(spr_enemy_face[i], HIDDEN);
            }
        }

        if (spr_int_life_counter != NULL) {
            SPR_setVisibility(spr_int_life_counter, HIDDEN);
        }

        for (u8 note = 0; note < 6; note++) {
            show_enemy_note(note + 1, false, false);
        }
    }

    SPR_update();
}

/**
 * Actualiza todas las máquinas de estado en combate.
 * Esta función debe llamarse en cada frame durante el combate.
 */
void combat_update(void)
{
    if (!is_combat_active) {
        return;
    }
    
    // Actualizar máquina de estado del jugador
    StateMachine_Update(&player_state_machine, NULL);
    
    // Actualizar máquinas de estado de enemigos
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active) {
            StateMachine_Update(&enemy_state_machines[i], NULL);
        }
    }
}
