#include "globals.h"
#include "patterns_registry.h"
#include "counter_spell.h"
#include "pattern_types/enemy_electric_pattern.h"
#include "pattern_types/enemy_bite_pattern.h"

// Referencia a la máquina de estados del jugador
extern StateMachine player_state_machine;

Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY];    // Stores all enemy pattern definitions
u16 enemy_attacking;                                   // Currently attacking enemy ID
u16 enemy_attack_pattern;                             // Current attack pattern type
u8 enemy_attack_pattern_notes;                        // Index in current note sequence
u16 enemy_attack_time;                                // Attack state timer
bool enemy_attack_effect_in_progress;                 // If attack effect is active
u16 enemy_attack_effect_time;                         // Effect duration timer
bool enemy_note_active[6];                            // Active note indicators
bool counter_spell_success = false;                   // Flag for successful counter-spell
u16 pending_counter_hit_enemy = ENEMY_NONE;           // Enemigo pendiente de recibir contraataque

// State machines for enemies
StateMachine enemy_state_machines[MAX_ENEMIES];       // State machines for each enemy

// Enemy patterns in the registry format
Pattern enemy_patterns[MAX_PATTERN_ENEMY];

void init_enemy_patterns(void)    // Setup enemy attack patterns and timings
{
    // Initialize old pattern system for compatibility
    // Make sure the notes array is properly initialized with all 4 values
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].numnotes = 4;
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[0] = 1;
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[1] = 2;
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[2] = 3;
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[3] = 4;
    obj_Pattern_Enemy[PTRN_EN_ELECTIC].recharge_time = 150;
    
    obj_Pattern_Enemy[PTRN_EN_BITE].numnotes = 3;
    obj_Pattern_Enemy[PTRN_EN_BITE].notes[0] = 2;
    obj_Pattern_Enemy[PTRN_EN_BITE].notes[1] = 3;
    obj_Pattern_Enemy[PTRN_EN_BITE].notes[2] = 2;
    obj_Pattern_Enemy[PTRN_EN_BITE].notes[3] = 0; // Padding
    obj_Pattern_Enemy[PTRN_EN_BITE].recharge_time = 150;
    
    // Debug log the pattern initialization
    kprintf("Enemy patterns initialized (explicit initialization):");
    kprintf("  Electric: %d notes [%d,%d,%d,%d], recharge: %d",
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].numnotes,
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[0],
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[1],
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[2],
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].notes[3],
            obj_Pattern_Enemy[PTRN_EN_ELECTIC].recharge_time);
    kprintf("  Bite: %d notes [%d,%d,%d,%d], recharge: %d",
            obj_Pattern_Enemy[PTRN_EN_BITE].numnotes,
            obj_Pattern_Enemy[PTRN_EN_BITE].notes[0],
            obj_Pattern_Enemy[PTRN_EN_BITE].notes[1],
            obj_Pattern_Enemy[PTRN_EN_BITE].notes[2],
            obj_Pattern_Enemy[PTRN_EN_BITE].notes[3],
            obj_Pattern_Enemy[PTRN_EN_BITE].recharge_time);
    
    // Initialize new pattern system
    enemy_patterns[PTRN_EN_ELECTIC] = (Pattern) {
        .id = PTRN_EN_ELECTIC,
        .active = true,
        .notes = {1,2,3,4},
        .owner_type = OWNER_ENEMY,
        .recharge_time = 150,
        .launch = enemy_electric_pattern_launch,
        .do_effect = enemy_electric_pattern_do,
        .finish = enemy_electric_pattern_finish,
        .sd = NULL
    };
    
    enemy_patterns[PTRN_EN_BITE] = (Pattern) {
        .id = PTRN_EN_BITE,
        .active = true,
        .notes = {2,3,2,0},
        .owner_type = OWNER_ENEMY,
        .recharge_time = 150,
        .launch = bite_pattern_launch,
        .do_effect = bite_pattern_do,
        .finish = bite_pattern_finish,
        .sd = NULL
    };
    
    // Register enemy patterns with the registry
    for (u8 i = 0; i < MAX_PATTERN_ENEMY; i++) {
        register_pattern(&enemy_patterns[i]);
    }
    
    // Initialize state machines for enemies
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        kprintf("Initializing state machine for enemy %d", i);
        StateMachine_Init(&enemy_state_machines[i], ENEMY_ENTITY_ID_BASE + i);
        enemy_state_machines[i].owner_type = OWNER_ENEMY;
        
        // Debug the initial state
        kprintf("  Enemy %d state machine initial state: %d",
                i, enemy_state_machines[i].current_state);
    }
}

/**
 * Enemy Pattern System
 * 
 * This module handles the enemy attack pattern system, including:
 * - Enemy attack state management
 * - Pattern execution and timing
 * - Visual and sound effects for attacks
 * - Pattern-specific behaviors (thunder, bite)
 * 
 * Enemy attacks flow through several states:
 * 1. STATE_PLAYING_NOTE: Playing attack notes sequence
 * 2. STATE_PATTERN_EFFECT: Executing attack effect
 * 3. STATE_PATTERN_EFFECT_FINISH: Concluding effect
 * 4. STATE_ATTACK_FINISHED: Post-attack cooldown
 * 5. STATE_IDLE: Ready for next attack
 */

void check_enemy_state(void)    // Main state machine for enemy pattern system
{
    u8 numenemy, npattern;
    u16 max_effect_time;
    
    // Process counter spell if still pending
    if (counter_spell_success && pending_counter_hit_enemy != ENEMY_NONE) {
        kprintf("Delayed counter-hit to enemy %d", pending_counter_hit_enemy);
        hit_enemy(pending_counter_hit_enemy);
        pending_counter_hit_enemy = ENEMY_NONE;
        counter_spell_success = false;
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_reversed = false;
        player_state_machine.pattern_system.effect_type = PTRN_NONE;
        player_state_machine.pattern_system.effect_in_progress = false;
        player_state_machine.pattern_system.effect_reversed = false;
        player_state_machine.pattern_system.effect_duration = 0;
    }

    // Log the current state every 60 frames
    if (frame_counter % 60 == 0) {
        kprintf("ENEMY STATE CHECK: enemy=%d, pattern=%d, effect_in_progress=%d",
                enemy_attacking, enemy_attack_pattern, enemy_attack_effect_in_progress);
    }
    
    // Update all enemy state machines
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        if (obj_enemy[i].obj_character.active) {
            // Debug state before update
            if (i == enemy_attacking) {
                kprintf("Enemy %d state BEFORE state machine update: %d",
                        i, obj_enemy[i].obj_character.state);
            }
            
            StateMachine_Update(&enemy_state_machines[i], NULL);
            
            // Debug state machine state
            if (i == enemy_attacking) {
                kprintf("Enemy %d state machine state AFTER update: %d",
                        i, enemy_state_machines[i].current_state);
            }
            
            // Update entity state from state machine
            if (i == enemy_attacking) {
                kprintf("Enemy %d state BEFORE applying state machine state: %d",
                        i, obj_enemy[i].obj_character.state);
            }
            
            update_character_from_sm_state(&obj_enemy[i].obj_character,
                                          enemy_state_machines[i].current_state);
            
            // Debug state after update
            if (i == enemy_attacking) {
                kprintf("Enemy %d state AFTER applying state machine state: %d",
                        i, obj_enemy[i].obj_character.state);
            }
        }
    }
    
    // We're now handling counter-spell cooldowns in counter_spell.c
    
    // Check for new attack opportunities when no attack is in progress
    if (!enemy_attack_effect_in_progress && enemy_attacking == ENEMY_NONE) {
        // Check if player is currently casting a spell or has an active effect
        bool player_is_casting = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                obj_character[active_character].state == STATE_PATTERN_CHECK ||
                                obj_character[active_character].state == STATE_PATTERN_EFFECT ||
                                obj_character[active_character].state == STATE_PATTERN_EFFECT_FINISH);
        
        // Check if player has an active counter-spell
        bool player_has_counter = (player_pattern_effect_in_progress != PTRN_NONE &&
                                 player_pattern_effect_reversed);
        
        // Log when we're checking for new attacks (less frequently)
        if (frame_counter % 180 == 0) {
            kprintf("CHECKING FOR NEW ATTACKS: player_casting=%d, player_counter=%d",
                    player_is_casting, player_has_counter);
        }
        
        // Only allow enemy attacks if player is not casting and has no counter active
        if (!player_is_casting && !player_has_counter) {
            for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
                if (obj_enemy[numenemy].obj_character.active) {
                    for (npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                        if (obj_enemy[numenemy].class.has_pattern[npattern]) {
                            // Check if pattern cooldown is complete
                            if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
                                if (player_pattern_effect_in_progress == PTRN_HIDE) {
                                    // Reduce cooldown if player is hidden
                                    obj_enemy[numenemy].last_pattern_time[npattern] -= 50;
                                } else {
                                    // Start new attack sequence using state machine
                                    enemy_attack_pattern_notes = 0;
                                    enemy_attack_time = 0;
                                    enemy_attack_pattern = npattern;
                                    enemy_attacking = numenemy;
                                    
                                    // Get pattern from registry
                                    Pattern* pattern = get_pattern(npattern, OWNER_ENEMY);
                                    
                                    // Configure state machine with pattern callbacks
                                    if (pattern) {
                                        enemy_state_machines[numenemy].launch_effect = pattern->launch;
                                        enemy_state_machines[numenemy].do_effect = pattern->do_effect;
                                        enemy_state_machines[numenemy].finish_effect = pattern->finish;
                                    }
                                    
                                    // Start with playing notes instead of going directly to effect
                                    // Maintain compatibility with existing code
                                    kprintf("Starting enemy attack sequence: enemy=%d, pattern=%d", numenemy, npattern);
                                    kprintf("Enemy state before: %d", obj_enemy[numenemy].obj_character.state);
                                    
                                    anim_enemy(numenemy, ANIM_ACTION);
                                    
                                    // Set the state in both the character and the state machine
                                    obj_enemy[numenemy].obj_character.state = STATE_PLAYING_NOTE;
                                    enemy_state_machines[numenemy].current_state = SM_STATE_PLAYING_NOTE;
                                    
                                    kprintf("Enemy state after setting to STATE_PLAYING_NOTE (%d): %d",
                                            STATE_PLAYING_NOTE,
                                            obj_enemy[numenemy].obj_character.state);
                                    kprintf("Enemy state machine state after setting: %d",
                                            enemy_state_machines[numenemy].current_state);
                                    
                                    // Prevent state machine from overriding our state
                                    enemy_state_machines[numenemy].note_time = 0;
                                    
                                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[0], true, true);
                                    show_or_hide_enemy_combat_interface(true);
                                    
                                    // Configure state machine with pattern callbacks
                                    // But don't send the pattern complete message yet - that happens after notes
                                }
                            } else {
                                // Random cooldown progression
                                if ((random() % 2) == 0) {
                                    obj_enemy[numenemy].last_pattern_time[npattern]++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    // Process active attack states
    if (enemy_attacking != ENEMY_NONE) {
        // Debug the enemy state less frequently (every 180 frames)
        if (frame_counter % 180 == 0) {
            kprintf("Enemy %d active attack state: %d, pattern: %d, note: %d/%d, time: %d",
                    enemy_attacking,
                    obj_enemy[enemy_attacking].obj_character.state,
                    enemy_attack_pattern,
                    enemy_attack_pattern_notes + 1,
                    obj_Pattern_Enemy[enemy_attack_pattern].numnotes,
                    enemy_attack_time);
        }
        
        switch (obj_enemy[enemy_attacking].obj_character.state)
        {
            case STATE_PLAYING_NOTE:
                // Handle note duration
                u16 max_time = calc_ticks(MAX_ATTACK_NOTE_PLAYING_TIME);
                
                // Debug the calculation and current state
                if (enemy_attack_time == 0) {
                    kprintf("ENEMY NOTE START: MAX_ATTACK_NOTE_PLAYING_TIME = %d ms, calculated ticks = %d",
                            MAX_ATTACK_NOTE_PLAYING_TIME, max_time);
                    kprintf("ENEMY NOTE INFO: enemy=%d, pattern=%d, note_index=%d, note_value=%d",
                            enemy_attacking,
                            enemy_attack_pattern,
                            enemy_attack_pattern_notes,
                            obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes]);
                }
                
                // Force the state machine to stay in PLAYING_NOTE state
                enemy_state_machines[enemy_attacking].current_state = SM_STATE_PLAYING_NOTE;
                
                // Safety check - if we've been in this state for too long, reset
                if (enemy_attack_time > max_time * 3) {
                    kprintf("WARNING: Enemy stuck in STATE_PLAYING_NOTE for too long, resetting");
                    obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
                    enemy_state_machines[enemy_attacking].current_state = SM_STATE_IDLE;
                    enemy_attacking = 254; // ENEMY_NONE
                    enemy_attack_effect_in_progress = false;
                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false, false);
                    show_or_hide_enemy_combat_interface(false);
                    break;
                }
                
                if (enemy_attack_time < max_time) {
                    enemy_attack_time++;
                    
                    // Log less frequently (every 10 frames)
                    if (enemy_attack_time % 10 == 0) {
                        kprintf("Enemy note playing time: %d/%d",
                                enemy_attack_time, max_time);
                    }
                } else {
                    // Current note finished
                    kprintf("Enemy note time reached max (%d frames), finishing note", max_time);
                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false, false);
                    
                    // Debug log with more details
                    kprintf("Enemy note %d finished, pattern has %d notes total",
                            enemy_attack_pattern_notes + 1,
                            obj_Pattern_Enemy[enemy_attack_pattern].numnotes);
                    
                    // Dump the entire pattern for debugging
                    kprintf("Full pattern: [%d,%d,%d,%d], current position: %d",
                            obj_Pattern_Enemy[enemy_attack_pattern].notes[0],
                            obj_Pattern_Enemy[enemy_attack_pattern].notes[1],
                            obj_Pattern_Enemy[enemy_attack_pattern].notes[2],
                            obj_Pattern_Enemy[enemy_attack_pattern].notes[3],
                            enemy_attack_pattern_notes);
                    
                    if (enemy_attack_pattern_notes < obj_Pattern_Enemy[enemy_attack_pattern].numnotes - 1) {
                        // Continue to next note in sequence
                        enemy_attack_pattern_notes++;
                        enemy_attack_time = 0;
                        
                        // Debug log with more details
                        kprintf("Playing next enemy note: %d (index %d of %d)",
                                obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes],
                                enemy_attack_pattern_notes + 1,
                                obj_Pattern_Enemy[enemy_attack_pattern].numnotes);
                        
                        // Make sure the note is valid
                        u8 next_note = obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes];
                        if (next_note >= 1 && next_note <= 6) {
                            show_enemy_note(next_note, true, true);
                        } else {
                            kprintf("ERROR: Invalid note value: %d", next_note);
                        }
                    } else {
                        // All notes complete, start effect
                        kprintf("All enemy notes complete (%d/%d), transitioning to effect phase",
                                enemy_attack_pattern_notes + 1,
                                obj_Pattern_Enemy[enemy_attack_pattern].numnotes);
                        
                        // Debug the pattern one more time
                        kprintf("Final pattern state: notes played = %d, pattern length = %d",
                                enemy_attack_pattern_notes + 1,
                                obj_Pattern_Enemy[enemy_attack_pattern].numnotes);
                        
                        // Debug the enemy state
                        kprintf("Enemy %d state before transition: %d",
                                enemy_attacking,
                                obj_enemy[enemy_attacking].obj_character.state);
                                
                        // Set both the entity state and the state machine state
                        obj_enemy[enemy_attacking].obj_character.state = STATE_PATTERN_EFFECT;
                        enemy_state_machines[enemy_attacking].current_state = SM_STATE_PATTERN_EFFECT;
                        
                        // Set the effect flags
                        enemy_attack_effect_in_progress = true;
                        enemy_attack_effect_time = 0;
                        enemy_state_machines[enemy_attacking].pattern_system.effect_in_progress = true;
                        enemy_state_machines[enemy_attacking].pattern_system.effect_type = enemy_attack_pattern;
                        enemy_state_machines[enemy_attacking].pattern_system.effect_duration = 0;
                        
                        // Debug the enemy state after transition
                        kprintf("Enemy %d state after transition: %d, state machine state: %d",
                                enemy_attacking,
                                obj_enemy[enemy_attacking].obj_character.state,
                                enemy_state_machines[enemy_attacking].current_state);
                        
                        // Get pattern from registry
                        Pattern* pattern = get_pattern(enemy_attack_pattern, OWNER_ENEMY);
                        
                        // Debug log
                        kprintf("Got pattern from registry: %s",
                                pattern ? "SUCCESS" : "FAILED");
                        
                        // Launch effect using pattern callbacks
                        if (pattern && pattern->launch) {
                            kprintf("Launching enemy pattern effect using registry callback");
                            pattern->launch(&enemy_state_machines[enemy_attacking]);
                        } else {
                            // Fallback to old system
                            kprintf("Using fallback (old system) for enemy pattern effect");
                            switch (enemy_attack_pattern) {
                                case PTRN_EN_ELECTIC:
                                    kprintf("Launching electric enemy pattern (fallback)");
                                    launch_electric_enemy_pattern();
                                    break;
                                case PTRN_EN_BITE:
                                    kprintf("Launching bite enemy pattern (fallback)");
                                    launch_bite_enemy_pattern();
                                    break;
                                default:
                                    kprintf("ERROR: Unknown enemy pattern type: %d", enemy_attack_pattern);
                                    break;
                            }
                        }
                    }
                    show_or_hide_enemy_combat_interface(true);
                }
                break;

            case STATE_PATTERN_EFFECT:
                // Force the state machine to stay in PATTERN_EFFECT state
                enemy_state_machines[enemy_attacking].current_state = SM_STATE_PATTERN_EFFECT;
                
                // Safety check - if we've been in this state for too long, reset
                if (enemy_attack_effect_time > 300) {  // ~5 seconds at 60fps
                    kprintf("WARNING: Enemy stuck in STATE_PATTERN_EFFECT for too long, resetting");
                    obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
                    enemy_state_machines[enemy_attacking].current_state = SM_STATE_IDLE;
                    enemy_attacking = 254; // ENEMY_NONE
                    enemy_attack_effect_in_progress = false;
                    VDP_setHilightShadow(false); // Ensure visual effects are disabled
                    show_or_hide_enemy_combat_interface(false);
                    break;
                }
                
                // Set effect duration based on pattern type
                switch (enemy_attack_pattern) {
                    case PTRN_EN_ELECTIC:
                        max_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC);
                        break;
                    case PTRN_EN_BITE:
                        max_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE);
                        break;
                    default:
                        max_effect_time = 100;
                }

                // Apply effect while duration not exceeded
                if (enemy_attack_effect_time < max_effect_time) {
                    // Log less frequently (every 60 frames)
                    if (enemy_attack_effect_time % 60 == 0) {
                        kprintf("Enemy effect in progress: %d/%d",
                                enemy_attack_effect_time,
                                max_effect_time);
                    }
                    
                    // Get pattern from registry
                    Pattern* pattern = get_pattern(enemy_attack_pattern, OWNER_ENEMY);
                    
                    // Process effect using pattern callbacks
                    if (pattern && pattern->do_effect) {
                        // Only log on first frame
                        if (enemy_attack_effect_time == 0) {
                            kprintf("Starting pattern effect for enemy %d, pattern %d",
                                    enemy_attacking, enemy_attack_pattern);
                        }
                        pattern->do_effect(&enemy_state_machines[enemy_attacking]);
                    } else {
                        // Fallback to old system
                        switch (enemy_attack_pattern) {
                            case PTRN_EN_ELECTIC:
                                do_electric_enemy_pattern_effect();
                                break;
                            case PTRN_EN_BITE:
                                do_bite_enemy_pattern_effect();
                                break;
                            default:
                                kprintf("ERROR: Unknown enemy pattern type in effect phase: %d", enemy_attack_pattern);
                                break;
                        }
                    }
                    enemy_attack_effect_time++;
                }

                // Check for effect completion
                if (enemy_attack_effect_time == max_effect_time) {
                    if (num_played_notes != 0) {
                        // Wait for player to finish their pattern
                        enemy_attack_effect_time--;
                    } else {
                        // Move to effect completion
                        obj_enemy[enemy_attacking].obj_character.state = STATE_PATTERN_EFFECT_FINISH;
                    }
                }
                break;

            case STATE_PATTERN_EFFECT_FINISH:
                // Clean up effect and transition to post-attack state
                // Get pattern from registry
                Pattern* pattern = get_pattern(enemy_attack_pattern, OWNER_ENEMY);
                
                // Finish effect using pattern callbacks
                if (pattern && pattern->finish) {
                    pattern->finish(&enemy_state_machines[enemy_attacking]);
                } else {
                    // Fallback to old system
                    finish_enemy_pattern_effect();
                }
                
                obj_enemy[enemy_attacking].obj_character.state = STATE_ATTACK_FINISHED;
                enemy_attack_time = 0;
                break;

            case STATE_ATTACK_FINISHED:
                // Wait in finished state before returning to idle
                if (enemy_attack_time < calc_ticks(MAX_TIME_AFTER_ATTACK)) {
                    enemy_attack_time++;
                } else {
                    obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
                    enemy_attacking = ENEMY_NONE;
                    show_or_hide_enemy_combat_interface(false);
                }
                break;

            default:
                break;
        }
    }
}

void show_enemy_note(u8 nnote, bool visible, bool play)    // Display/play enemy note (1-6:MI-DO), manage sprites and sound
{
    Sprite **rodsprite;
    const SpriteDefinition *rodspritedef;
    const u8 *notesong;
    u16 rod_x;

    // Configure sprite and sound based on note
    switch (nnote) 
    {
    case NOTE_MI:
        rodsprite = &spr_int_enemy_rod_1;
        rodspritedef = &int_enemy_rod_1_sprite;
        notesong = snd_enemy_note_mi;
        rod_x = 24;
        break;
    case NOTE_FA:
        rodsprite = &spr_int_enemy_rod_2;
        rodspritedef = &int_enemy_rod_2_sprite;
        notesong = snd_enemy_note_fa;
        rod_x = 24 + 32;
        break;
    case NOTE_SOL:
        rodsprite = &spr_int_enemy_rod_3;
        rodspritedef = &int_enemy_rod_3_sprite;
        notesong = snd_enemy_note_sol;
        rod_x = 24 + 64;
        break;
    case NOTE_LA:
        rodsprite = &spr_int_enemy_rod_4;
        rodspritedef = &int_enemy_rod_4_sprite;
        notesong = snd_enemy_note_la;
        rod_x = 24 + 96;
        break;
    case NOTE_SI:
        rodsprite = &spr_int_enemy_rod_5;
        rodspritedef = &int_enemy_rod_5_sprite;
        notesong = snd_enemy_note_si;
        rod_x = 24 + 128;
        break;
    default: // NOTE_DO
        rodsprite = &spr_int_enemy_rod_6;
        rodspritedef = &int_enemy_rod_6_sprite;
        notesong = snd_enemy_note_do;
        rod_x = 24 + 160;
        break;
    }

    if (visible) {
        // Show note sprite and play sound
        if (*rodsprite == NULL) {
            *rodsprite = SPR_addSpriteSafe(rodspritedef, rod_x, 184, TILE_ATTR(PAL2, false, false, false));
            if (*rodsprite == NULL) return;
        }
        SPR_setVisibility(*rodsprite, VISIBLE);
        enemy_note_active[nnote - 1] = true;
        if (play) play_music(notesong);
    } else {
        // Hide and cleanup note sprite
        if (*rodsprite != NULL) {
            SPR_releaseSprite(*rodsprite);
            *rodsprite = NULL;
            enemy_note_active[nnote - 1] = false;
        }
    }
}

void finish_enemy_pattern_effect(void)    // Clean up and finish enemy attack pattern effects
{
    kprintf("FINISH_ENEMY_PATTERN_EFFECT called: enemy=%d, pattern=%d",
            enemy_attacking, enemy_attack_pattern);
    
    // Reset enemy state
    anim_enemy(enemy_attacking, ANIM_IDLE);
    
    // Set the cooldown to 50% of the recharge time to allow the enemy to attack again after a delay
    // This is less than the 75% used for counter-spells, so enemies will attack sooner after normal attacks
    if (enemy_attacking != ENEMY_NONE && enemy_attack_pattern != PTRN_EN_NONE) {
        obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = obj_Pattern_Enemy[enemy_attack_pattern].recharge_time / 2;
        kprintf("  - Set cooldown for enemy %d pattern %d to %d",
                enemy_attacking, enemy_attack_pattern, obj_Pattern_Enemy[enemy_attack_pattern].recharge_time / 2);
    }
    
    enemy_attack_effect_time = 0;
    enemy_attack_effect_in_progress = false;

    // Apply final pattern effects
    switch (enemy_attack_pattern) {
        case PTRN_EN_ELECTIC:
            finish_electric_enemy_pattern_effect();
            break;
        case PTRN_EN_BITE:
            finish_bite_enemy_pattern_effect();
            break;
    }
}

/******************************************************************************
 *                        Pattern-Specific Effects                             *
 ******************************************************************************/

// These functions are now implemented in pattern_types/enemy_electric_pattern.c
void launch_electric_enemy_pattern(void) { /* Deprecated */ }
void do_electric_enemy_pattern_effect(void) { /* Deprecated */ }
void finish_electric_enemy_pattern_effect(void) { /* Deprecated */ }

// These functions are now implemented in pattern_types/bite_pattern.c
void launch_bite_enemy_pattern(void) { /* Deprecated */ }
void do_bite_enemy_pattern_effect(void) { /* Deprecated */ }
void finish_bite_enemy_pattern_effect(void) { /* Deprecated */ }

void cleanup_enemy_notes(void)    // Clean up all active enemy notes when resetting combat
{
    // Clean up all note sprites and states
    for (u8 note = 0; note < 6; note++) {
        if (enemy_note_active[note]) {
            show_enemy_note(note + 1, false, false);
            enemy_note_active[note] = false;
        }
    }
}
