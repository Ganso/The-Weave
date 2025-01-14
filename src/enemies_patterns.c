#include <genesis.h>
#include "globals.h"

// Global variable definitions
Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY];
u16 enemy_attacking;
u16 enemy_attack_pattern;
u8 enemy_attack_pattern_notes;
u16 enemy_attack_time;
bool enemy_attack_effect_in_progress;
u16 enemy_attack_effect_time; 
bool enemy_note_active[6];


// Initialize enemy pattern
void init_enemy_patterns(void)
{
    obj_Pattern_Enemy[PTRN_EN_ELECTIC]=(Pattern_Enemy) {4, {1,2,3,4}, 150}; // Electric pattern: 4 steps, 150ms interval
    obj_Pattern_Enemy[PTRN_EN_BITE]=(Pattern_Enemy) {3, {2,3,2,NULL}, 150}; // Bite pattern: 3 steps, 150ms interval
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

/**
 * Main state machine for enemy pattern system
 * Controls enemy attack patterns, timing, and effects
 */
void check_enemy_state(void)
{
    u8 numenemy, npattern;
    u16 max_effect_time;

    // Check for new attack opportunities when no attack is in progress
    if (!enemy_attack_effect_in_progress && enemy_attacking == ENEMY_NONE) {
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
                                // Start new attack sequence
                                enemy_attack_pattern_notes = 0;
                                enemy_attack_time = 0;
                                enemy_attack_pattern = npattern;
                                enemy_attacking = numenemy;
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

/**
 * Display and optionally play an enemy note
 * Manages note sprites and sound effects
 * 
 * @param nnote: Note to display (1-6 for MI through DO)
 * @param visible: Whether to show or hide the note
 * @param play: Whether to play the note sound
 */
void show_enemy_note(u8 nnote, bool visible, bool play)
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

/**
 * Conclude an enemy attack pattern
 * Handles cleanup and final effects
 */
void finish_enemy_pattern_effect(void) {
    // Reset enemy state
    anim_enemy(enemy_attacking, ANIM_IDLE);
    obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = 0;
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

/**
 * Initialize electric pattern attack
 */
void launch_electric_enemy_pattern(void) {
    play_pattern_sound(PTRN_ELECTRIC);
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

/**
 * Process ongoing electric pattern effect
 * Handles visual effects and counter-spell interaction
 */
void do_electric_enemy_pattern_effect(void) {
    // Create lightning flash effect
    if (frame_counter % 2 == 0) VDP_setHilightShadow(true);
    else VDP_setHilightShadow(false);

    // Check for player counter-spell
    if (player_pattern_effect_in_progress == PTRN_ELECTRIC && player_pattern_effect_reversed == true) {
        VDP_setHilightShadow(false);
        hit_enemy(enemy_attacking);
        
        // Clean up all active notes
        cleanup_enemy_notes();
        
        // Reset enemy state
        anim_enemy(enemy_attacking, ANIM_IDLE);
        enemy_attack_effect_in_progress = false;
        obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
        obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = 0;
        enemy_attacking = ENEMY_NONE;
        
        // Reset player state
        player_pattern_effect_in_progress = PTRN_NONE;
        player_pattern_effect_reversed = false;
        obj_character[active_character].state = STATE_IDLE;
        
        // Hide combat interface
        show_or_hide_enemy_combat_interface(false);
    }
}

/**
 * Complete electric pattern effect
 * Applies damage if player failed to counter
 */
void finish_electric_enemy_pattern_effect(void) {
    VDP_setHilightShadow(false);
    if (enemy_attacking != ENEMY_NONE) {
        // Player failed to counter
        hit_caracter(active_character);
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar al revés" - (EN) "I should maybe think backwards"
        show_or_hide_interface(true);
        show_or_hide_enemy_combat_interface(true);
    }
}

/**
 * Initialize bite pattern attack
 */
void launch_bite_enemy_pattern(void) {
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

/**
 * Process ongoing bite pattern effect
 * Checks for player hide state
 */
void do_bite_enemy_pattern_effect(void) {
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE) - 1;
    }
}

/**
 * Complete bite pattern effect
 * Applies damage if player is not hidden
 */
void finish_bite_enemy_pattern_effect(void) {
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
    talk_dialog(&dialogs[ACT1_DIALOG3][4]); // (ES) "Puedo probar a esconderme o tratar de invocar al trueno" - (EN) "I could try to hide or attempt to summon the thunder"
    show_or_hide_interface(true);
    show_or_hide_enemy_combat_interface(true);
    show_character(active_character, true);
}

/**
 * Clean up all active enemy notes
 * Used when resetting combat state
 */
void cleanup_enemy_notes(void) {
    // Clean up all note sprites and states
    for (u8 note = 0; note < 6; note++) {
        if (enemy_note_active[note]) {
            show_enemy_note(note + 1, false, false);
            enemy_note_active[note] = false;
        }
    }
}
