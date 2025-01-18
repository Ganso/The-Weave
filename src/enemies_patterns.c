#include <genesis.h>
#include "globals.h"

// Enemy pattern definitions
Pattern_Enemy obj_Pattern_Enemy[MAX_PATTERN_ENEMY];

// Enemy pattern state machine instance
static EnemyPatternStateMachine enemy_pattern_sm;

// Note indicator states
bool enemy_note_active[6];

// Currently attacking enemy tracking
u16 enemy_attacking = ENEMY_NONE;
u16 enemy_attack_pattern = 0;


void init_enemy_patterns(void)    // Setup enemy attack patterns and timings
{
    // Initialize pattern definitions
    obj_Pattern_Enemy[PTRN_EN_ELECTIC]=(Pattern_Enemy) {4, {1,2,3,4}, 150}; // Electric pattern: 4 steps, 150ms interval
    obj_Pattern_Enemy[PTRN_EN_BITE]=(Pattern_Enemy) {3, {2,3,2,NULL}, 150}; // Bite pattern: 3 steps, 150ms interval
    
    // Initialize pattern state machine
    enemy_pattern_sm_init(&enemy_pattern_sm, DEBUG_STATE_MACHINES);
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
    // Update pattern state machine
    enemy_pattern_sm_update(&enemy_pattern_sm);
    
    // Only check for new attacks when no pattern is active
    if (!enemy_pattern_sm_is_active(&enemy_pattern_sm)) {
        // Check each enemy for pattern opportunities
        for (u8 numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active) {
                for (u8 npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern]) {
                        // Check if pattern cooldown is complete
                        if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
                            if (player_pattern_effect_in_progress == PTRN_HIDE) {
                                // Reduce cooldown if player is hidden
                                obj_enemy[numenemy].last_pattern_time[npattern] -= 50;
                            } else {
                                // Start new pattern
                                enemy_attacking = numenemy;
                                enemy_attack_pattern = npattern;
                                anim_enemy(numenemy, ANIM_ACTION);
                                enemy_pattern_sm_start(&enemy_pattern_sm, numenemy, npattern);
                                show_or_hide_enemy_combat_interface(true);
                                break; // Only start one pattern at a time
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
    // Reset enemy state
    anim_enemy(enemy_attacking, ANIM_IDLE);
    obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = 0;
    combat_sm.effect_timer = 0;
    combat_sm.effect_in_progress = false;

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
        combat_sm.effect_in_progress = false;
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

void finish_electric_enemy_pattern_effect(void)    // Complete electric pattern and apply damage if not countered
{
    VDP_setHilightShadow(false);
    if (enemy_attacking != ENEMY_NONE) {
        // Player failed to counter
        hit_character(active_character);
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][2]); // (ES) "Eso ha dolido" - (EN) "That hurts"
        talk_dialog(&dialogs[ACT1_DIALOG3][3]); // (ES) "Quizá deba pensar al revés" - (EN) "I should maybe think backwards"
        show_or_hide_interface(true);
        show_or_hide_enemy_combat_interface(true);
    }
}

void launch_bite_enemy_pattern(void)    // Start bite pattern attack sequence
{
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

void do_bite_enemy_pattern_effect(void)    // Process bite pattern effect and check player hide state
{
    if (player_pattern_effect_in_progress == PTRN_HIDE) {
        combat_sm.effect_timer = calc_ticks(MAX_EFFECT_TIME_BITE) - 1;
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
    talk_dialog(&dialogs[ACT1_DIALOG3][4]); // (ES) "Puedo probar a esconderme o tratar de invocar al trueno" - (EN) "I could try to hide or attempt to summon the thunder"
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
