#include "globals.h"
#include "combat.h"

// Debug output macro
#define COMBAT_DEBUG(fmt, ...) \
    if (DEBUG_STATE_MACHINES) { \
        kprintf("[COMBAT] " fmt "\n", ##__VA_ARGS__); \
    }

// Combat state machine instance
CombatStateMachine combat_sm;

void start_combat(bool start)    // Initialize or cleanup combat sequence with UI and enemy states
{
    u8 numenemy, npattern;

    if (start) {
        COMBAT_DEBUG("Starting combat sequence");
        
        // Initialize combat state machine if not already done
        static bool sm_initialized = false;
        if (!sm_initialized) {
            combat_sm_init(&combat_sm, DEBUG_STATE_MACHINES);
            sm_initialized = true;
            COMBAT_DEBUG("Combat state machine initialized");
        }

        // Combat initialization
        setRandomSeed(frame_counter);
        player_scroll_active = false;

        // Reset enemies to combat-ready state
        for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active) {
                COMBAT_DEBUG("Initializing enemy %d", numenemy);
                
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

        // Setup combat UI
        spr_int_life_counter = SPR_addSprite(&int_life_counter_sprite, 164, 180, TILE_ATTR(PAL2, false, false, false));
        SPR_setVisibility(spr_int_life_counter, HIDDEN);

        // Start combat state machine
        combat_sm_start(&combat_sm, enemy_attacking);
    }
    else {
        COMBAT_DEBUG("Ending combat sequence");
        
        // End combat state machine
        combat_sm_end(&combat_sm);
        
        // Combat cleanup
        player_scroll_active = true;
        
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

    COMBAT_DEBUG("Enemy %d hit", nenemy);

    // Play hit sound effect
    play_sample(snd_player_hit_enemy, sizeof(snd_player_hit_enemy));

    // If enemy was attacking, interrupt their pattern
    if (enemy_attacking == nenemy) {
        COMBAT_DEBUG("Interrupting enemy %d pattern", nenemy);
        
        // Clean up all active notes
        cleanup_enemy_notes();
        
        // Reset enemy state
        anim_enemy(nenemy, ANIM_IDLE);
        obj_enemy[nenemy].obj_character.state = STATE_IDLE;
        obj_enemy[nenemy].last_pattern_time[enemy_attack_pattern] = 0;
        enemy_attacking = ENEMY_NONE;
        
        // Interrupt enemy pattern cast
        combat_sm_interrupt_cast(&combat_sm, nenemy);
    }

    // Apply damage and check for defeat
    obj_enemy[nenemy].hitpoints--;
    if (obj_enemy[nenemy].hitpoints == 0) {
        COMBAT_DEBUG("Enemy %d defeated", nenemy);
        
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
            COMBAT_DEBUG("All enemies defeated, ending combat");
            start_combat(false);  // End combat when all enemies defeated
        }
    }
    else {
        COMBAT_DEBUG("Enemy %d HP: %d", nenemy, obj_enemy[nenemy].hitpoints);
        
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

    // Send enemy hit message
    msg_send(MSG_ENEMY_HIT, nenemy, obj_enemy[nenemy].hitpoints, NULL);
}

void hit_character(u16 nchar)    // Handle player character damage (currently just sound)
{
    COMBAT_DEBUG("Character %d hit", nchar);
    
    play_sample(snd_player_hurt, sizeof(snd_player_hurt));
    
    // Send player hit message
    msg_send(MSG_PLAYER_HIT, nchar, 0, NULL);
}

void show_or_hide_enemy_combat_interface(bool show)    // Toggle combat UI elements (faces, life counter, notes)
{
    if (show && interface_active && combat_sm.is_active && enemy_attacking != ENEMY_NONE) {
        COMBAT_DEBUG("Showing combat interface for enemy %d", enemy_attacking);
        
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
        COMBAT_DEBUG("Hiding combat interface");
        
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
