#include <genesis.h>
#include "globals.h"

// Global variable definitions
bool is_combat_active;

/**
 * Combat System
 * 
 * This module handles the core combat mechanics including:
 * - Combat initialization and cleanup
 * - Damage calculation and application
 * - Combat UI management (life counters, enemy faces, note indicators)
 * - State tracking for active combats
 */

/******************************************************************************
 *                            Combat Management                                 *
 ******************************************************************************/

/**
 * Initialize or cleanup a combat sequence
 * 
 * When starting combat:
 * - Initializes RNG for unpredictable combat events
 * - Resets enemy health and randomizes initial pattern timings
 * - Sets up combat UI elements
 * - Disables player scrolling during combat
 * 
 * When ending combat:
 * - Cleans up UI elements
 * - Re-enables player movement
 * 
 * @param start: true to start combat, false to end it
 */
void start_combat(bool start)
{
    u8 numenemy, npattern, nnote;

    if (start) {
        // Combat initialization
        setRandomSeed(frame_counter);
        is_combat_active = true;
        player_scroll_active = false;

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
        }

        // Initialize combat state
        enemy_attacking = ENEMY_NONE;
        enemy_attack_pattern_notes = 0;
        enemy_attack_time = 0;
        enemy_attack_effect_in_progress = false;
        for (nnote = 0; nnote < 6; nnote++) {
            enemy_note_active[nnote] = false;
        }

        // Setup combat UI
        spr_int_life_counter = SPR_addSprite(&int_life_counter_sprite, 164, 180, TILE_ATTR(PAL2, false, false, false));
        SPR_setVisibility(spr_int_life_counter, HIDDEN);
    }
    else {
        // Combat cleanup
        is_combat_active = false;
        player_scroll_active = true;
        
        // Cleanup life counter sprite
        if (spr_int_life_counter != NULL) {
            SPR_releaseSprite(spr_int_life_counter);
            spr_int_life_counter = NULL;
        }
    }
}

/******************************************************************************
 *                            Damage Processing                                 *
 ******************************************************************************/

/**
 * Process damage dealt to an enemy
 * 
 * - Reduces enemy HP by 1
 * - Handles enemy defeat if HP reaches 0
 * - Updates combat UI to show damage
 * - Checks for combat end if all enemies defeated
 * 
 * @param nenemy: Index of the enemy taking damage
 */
void hit_enemy(u16 nenemy)
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
        obj_enemy[nenemy].last_pattern_time[enemy_attack_pattern] = 0;
        enemy_attacking = ENEMY_NONE;
        
        // Hide combat interface
        show_or_hide_enemy_combat_interface(false);
    }

    // Apply damage and check for defeat
    obj_enemy[nenemy].hitpoints--;
    if (obj_enemy[nenemy].hitpoints == 0) {
        // Enemy defeated
        SPR_setVisibility(spr_int_life_counter, HIDDEN);
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

/**
 * Process damage dealt to a player character
 * 
 * Currently only plays hurt sound effect.
 * TODO: Implement actual damage system for player characters
 * 
 * @param nchar: Index of the character taking damage
 */
void hit_caracter(u16 nchar)
{
    play_sample(snd_player_hurt,sizeof(snd_player_hurt));
}

/******************************************************************************
 *                            Combat UI Management                             *
 ******************************************************************************/

/**
 * Update visibility of enemy combat interface elements
 * 
 * When showing interface (show=true):
 * - Shows attacking enemy's face
 * - Hides other enemy faces
 * - Updates life counter for attacking enemy
 * - Shows active note indicators
 * 
 * When hiding interface (show=false):
 * - Hides all enemy faces
 * - Hides life counter
 * - Hides all note indicators
 * 
 * @param show: true to show interface, false to hide it
 */
void show_or_hide_enemy_combat_interface(bool show)
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
