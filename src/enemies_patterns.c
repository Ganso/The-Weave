#include "globals.h"

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

// State machines for enemies
StateMachine enemy_state_machines[MAX_ENEMIES];       // State machines for each enemy

void init_enemy_patterns(void)    // Setup enemy attack patterns and timings
{
    obj_Pattern_Enemy[PTRN_EN_ELECTIC]=(Pattern_Enemy) {4, {1,2,3,4}, 150}; // Electric pattern: 4 steps, 150ms interval
    obj_Pattern_Enemy[PTRN_EN_BITE]=(Pattern_Enemy) {3, {2,3,2,NULL}, 150}; // Bite pattern: 3 steps, 150ms interval
    
    // Initialize state machines for enemies
    for (u8 i = 0; i < MAX_ENEMIES; i++) {
        StateMachine_Init(&enemy_state_machines[i], ENEMY_ENTITY_ID_BASE + i);
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
    
    // Reset counter-spell success flag at the start of each frame
    // This allows new attacks to start after a counter-spell has been processed
    counter_spell_success = false;

    // Log the current state every 60 frames
    if (frame_counter % 60 == 0) {
        kprintf("ENEMY STATE CHECK: enemy=%d, pattern=%d, effect_in_progress=%d",
                enemy_attacking, enemy_attack_pattern, enemy_attack_effect_in_progress);
    }
    
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
        
        // Log when we're checking for new attacks
        if (frame_counter % 60 == 0) {
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
                                    
                                    // Enviar mensaje a la máquina de estados del enemigo
                                    StateMachine_SendMessage(&enemy_state_machines[numenemy],
                                                           MSG_NOTE_PLAYED,
                                                           obj_Pattern_Enemy[npattern].notes[0]);
                                    
                                    // Mantener compatibilidad con el código existente
                                    anim_enemy(numenemy, ANIM_ACTION);
                                    obj_enemy[numenemy].obj_character.state = STATE_PLAYING_NOTE;
                                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[0], true, true);
                                    show_or_hide_enemy_combat_interface(true);
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
        switch (obj_enemy[enemy_attacking].obj_character.state)
        {
            case STATE_PLAYING_NOTE:
                // Handle note duration
                if (enemy_attack_time != calc_ticks(MAX_ATTACK_NOTE_PLAYING_TIME)) {
                    enemy_attack_time++;
                } else {
                    // Current note finished
                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false, false);
                    
                    if (enemy_attack_pattern_notes != obj_Pattern_Enemy[enemy_attack_pattern].numnotes - 1) {
                        // Continue to next note in sequence
                        enemy_attack_pattern_notes++;
                        enemy_attack_time = 0;
                        show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true, true);
                    } else {
                        // All notes complete, start effect
                        obj_enemy[enemy_attacking].obj_character.state = STATE_PATTERN_EFFECT;
                        enemy_attack_effect_in_progress = true;
                        enemy_attack_effect_time = 0;
                        
                        // Initialize pattern-specific effect
                        switch (enemy_attack_pattern) {
                            case PTRN_EN_ELECTIC:
                                launch_electric_enemy_pattern();
                                break;
                            case PTRN_EN_BITE:
                                launch_bite_enemy_pattern();
                                break;
                        }
                    }
                    show_or_hide_enemy_combat_interface(true);
                }
                break;

            case STATE_PATTERN_EFFECT:
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
                    switch (enemy_attack_pattern) {
                        case PTRN_EN_ELECTIC:
                            do_electric_enemy_pattern_effect();
                            break;
                        case PTRN_EN_BITE:
                            do_bite_enemy_pattern_effect();
                            break;
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
                finish_enemy_pattern_effect();
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

void launch_electric_enemy_pattern(void)    // Start electric pattern attack sequence
{
    play_pattern_sound(PTRN_ELECTRIC);
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

void do_electric_enemy_pattern_effect(void)    // Process electric pattern effect and counter-spell checks
{
    // If counter-spell already succeeded, don't do anything
    if (counter_spell_success) {
        return;
    }
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    // Create lightning flash effect
    if (frame_counter % 2 == 0) VDP_setHilightShadow(true);
    else VDP_setHilightShadow(false);

    // Check for player counter-spell
    if (player_pattern_effect_in_progress == PTRN_ELECTRIC && player_pattern_effect_reversed == true) {
        // Set the counter-spell success flag
        counter_spell_success = true;
        
        // Stop all visual effects immediately
        VDP_setHilightShadow(false);
        
        // Show success message
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "¡Contraataque!" - (EN) "Counter-attack!"
        show_or_hide_enemy_combat_interface(true);
        show_or_hide_interface(true);
        
        // Damage the enemy
        hit_enemy(enemy_attacking);
        
        // Clean up all active notes
        cleanup_enemy_notes();
        
        // Reset enemy state
        anim_enemy(enemy_attacking, ANIM_IDLE);
        enemy_attack_effect_in_progress = false;
        obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
        
        // Set the cooldown to 75% of the recharge time to allow the enemy to attack again after a delay
        u16 current_enemy = enemy_attacking;
        u16 current_pattern = enemy_attack_pattern;
        obj_enemy[current_enemy].last_pattern_time[current_pattern] = obj_Pattern_Enemy[current_pattern].recharge_time * 3 / 4;
        
        // Reset attack state
        enemy_attack_pattern = PTRN_EN_NONE;
        enemy_attacking = ENEMY_NONE;
        
        // Reset player state
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_reversed = false;
        player_pattern_effect_time = 0;
        obj_character[active_character].state = STATE_IDLE;
        anim_character(active_character, ANIM_IDLE);
        
        // Hide combat interface
        show_or_hide_enemy_combat_interface(false);
        
        // Reset any other active effects
        for (u16 i = 0; i < MAX_ENEMIES; i++) {
            if (obj_enemy[i].obj_character.active) {
                // Reset any enemy in pattern effect state
                if (obj_enemy[i].obj_character.state == STATE_PATTERN_EFFECT ||
                    obj_enemy[i].obj_character.state == STATE_PATTERN_EFFECT_FINISH) {
                    obj_enemy[i].obj_character.state = STATE_IDLE;
                    anim_enemy(i, ANIM_IDLE);
                }
            }
        }
        
        // Hide any pattern icons
        show_pattern_icon(PTRN_ELECTRIC, false, false);
        
        // Update sprites to reflect changes
        SPR_update();
    }
    
    // If player is playing notes, extend the effect time to give them a chance to counter
    if (player_is_playing_notes && enemy_attack_effect_time >= calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30) {
        // Keep the effect going a bit longer
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
        kprintf("Player is playing notes - extending enemy attack time");
    }
}

void finish_electric_enemy_pattern_effect(void)    // Complete electric pattern and apply damage if not countered
{
    kprintf("FINISH_ELECTRIC_ENEMY_PATTERN_EFFECT called: enemy=%d, counter_success=%d",
            enemy_attacking, counter_spell_success);
    
    // If counter-spell already succeeded, don't do anything
    if (counter_spell_success) {
        kprintf("Counter spell already successful, skipping damage and dialog");
        VDP_setHilightShadow(false);
        return;
    }
    
    VDP_setHilightShadow(false);
    
    // Check if player is currently playing notes
    bool player_is_playing_notes = (obj_character[active_character].state == STATE_PLAYING_NOTE ||
                                  num_played_notes > 0);
    
    if (enemy_attacking != ENEMY_NONE) {
        if (player_is_playing_notes) {
            // Player is still trying to counter, give them more time
            kprintf("Player is still playing notes - delaying damage");
            
            // Extend the effect time to give them a chance to counter
            enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 30;
            
            // Don't apply damage yet
            return;
        } else {
            // Player failed to counter
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
        }
    }
    
    kprintf("FINISH_ELECTRIC_ENEMY_PATTERN_EFFECT completed");
}

void launch_bite_enemy_pattern(void)    // Start bite pattern attack sequence
{
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

void do_bite_enemy_pattern_effect(void)    // Process bite pattern effect and check player hide state
{
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE) - 1;
    }
}

void finish_bite_enemy_pattern_effect(void)    // Complete bite pattern and apply damage if player not hidden
{
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        // Player successfully avoided attack
        return;
    }

    // Player failed to hide
    show_character(active_character, true);
    hit_caracter(active_character);
    show_or_hide_interface(false);
    show_or_hide_enemy_combat_interface(false);
    talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
    talk_dialog(&dialogs[ACT1_DIALOG3][4]); // (ES) "Puedo probar a esconderme|o tratar de invocar|al trueno" - (EN) "I could try to hide|or attempt to summon|the thunder"
    show_or_hide_interface(true);
    show_or_hide_enemy_combat_interface(true);
    show_character(active_character, true);
}

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

