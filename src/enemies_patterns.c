#include <genesis.h>
#include "globals.h"

// Initiate an enemy attack pattern
void enemy_launch_pattern(u8 numenemy, u8 npattern)
{
    if (enemy_attacking==ENEMY_NONE) { // Ensure no other enemy is currently attacking
        enemy_attack_pattern_notes=0;
        enemy_attack_time=0;
        enemy_attack_pattern=npattern;
        enemy_attacking=numenemy;
        anim_enemy(numenemy,ANIM_ACTION); // Set enemy to action animation
        enemy_launch_pattern_note(); // Start playing the pattern notes
        show_or_hide_enemy_combat_interface(true); // Show combat UI for this enemy
    }
}

// Play the next note in the enemy's attack pattern
void enemy_launch_pattern_note()
{
    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true, true);
}

// Display and optionally play a specific enemy note
void show_enemy_note(u8 nnote, bool visible, bool play)
{
    Sprite **rodsprite;
    const SpriteDefinition *rodspritedef;
    const u8 *notesong;
    u16 rod_x;

    // Select the appropriate sprite and sound for the note
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
        // Create sprite if it doesn't exist
        if (*rodsprite == NULL) {
            *rodsprite = SPR_addSpriteSafe(rodspritedef, rod_x, 184, TILE_ATTR(PAL2, false, false, false));
            if (*rodsprite == NULL) {
                // Handle sprite creation failure
                return;
            }
        }
        SPR_setVisibility(*rodsprite, VISIBLE);
        enemy_note_active[nnote - 1] = true;
        if (play) {
            XGM2_play(notesong); // Play the note sound
        }
    } else {
        // Hide and release the sprite if it exists
        if (*rodsprite != NULL) {
            SPR_releaseSprite(*rodsprite);
            *rodsprite = NULL;
            enemy_note_active[nnote - 1] = false;
        }
    }
}

// Check if any enemy should initiate an attack pattern
void check_enemy_pattern(void)
{
    u8 numenemy, npattern;

    // Only check for new patterns if no attack is currently in progress
    if (enemy_attack_effect_in_progress == false && enemy_attacking == ENEMY_NONE) {
        for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active == true) {
                for (npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern] == true) {
                        // Check pattern-specific conditions for each enemy
                        switch (npattern) {
                            case PTRN_EN_ELECTIC:
                                check_electric_pattern(numenemy, npattern);
                                break;
                            case PTRN_EN_BITE:
                                check_bite_pattern(numenemy, npattern);
                                break;
                            // Add more cases for other pattern types as needed
                        }
                    }
                }
            }
        }
    }

    // Handle ongoing attack if one is in progress
    if (enemy_attack_effect_in_progress == false && enemy_attacking != ENEMY_NONE) {
        handle_ongoing_attack();
    }

    // Apply attack effect if it's in progress
    if (enemy_attack_effect_in_progress == true) {
        do_enemy_pattern_effect();
    }
}

// Process the ongoing enemy attack
void handle_ongoing_attack(void)
{
    if (enemy_attack_time != calc_ticks(MAX_ATTACK_NOTE_PLAYING_TIME)) {
        enemy_attack_time++; // Continue playing current note
    } else {
        // Current note finished playing
        show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false, false);
        
        if (enemy_attack_pattern_notes != obj_Pattern_Enemy[enemy_attack_pattern].numnotes - 1) {
            // Play next note in the pattern
            enemy_attack_pattern_notes++;
            enemy_attack_time = 0;
            show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true, true);
        } else {
            // All notes played, initiate pattern effect
            enemy_attack_effect_in_progress = true;
            enemy_attack_effect_time = 0;
            
            // Launch pattern-specific effect
            switch (enemy_attack_pattern) {
                case PTRN_EN_ELECTIC:
                    launch_electric_pattern();
                    break;
                case PTRN_EN_BITE:
                    launch_bite_pattern();
                    break;
                // Add more cases for other pattern types as needed
            }
        }
        show_or_hide_enemy_combat_interface(true);
    }
}

// Apply the effect of the enemy's attack pattern
void do_enemy_pattern_effect(void) {
    u16 max_effect_time;

    // Determine max effect duration based on pattern type
    switch (enemy_attack_pattern) {
        case PTRN_EN_ELECTIC:
            max_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC);
            break;
        case PTRN_EN_BITE:
            max_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE);
            break;
        default:
            max_effect_time = 100; // Default duration
    }

    if (enemy_attack_effect_time < max_effect_time) {
        // Apply pattern-specific effect
        switch (enemy_attack_pattern) {
            case PTRN_EN_ELECTIC:
                do_electric_pattern_effect();
                break;
            case PTRN_EN_BITE:
                do_bite_pattern_effect();
                break;
            // Add more cases for other pattern types as needed
        }
        enemy_attack_effect_time++;
    }

    if (enemy_attack_effect_time == max_effect_time) {
        if (num_played_notes != 0) {
            // Player is mid-pattern, give them time to finish
            enemy_attack_effect_time--;
        } else {
            // Conclude the attack
            finish_enemy_pattern_effect();
        }
    }    
}

// Conclude the enemy's attack pattern and apply final effects
void finish_enemy_pattern_effect(void) {
    anim_enemy(enemy_attacking, ANIM_IDLE);
    obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = 0;
    enemy_attack_effect_time = 0;
    enemy_attack_effect_in_progress = false;

    // Apply pattern-specific conclusion effects
    switch (enemy_attack_pattern) {
        case PTRN_EN_ELECTIC:
            finish_electric_pattern_effect();
            break;
        case PTRN_EN_BITE:
            finish_bite_pattern_effect();
            break;
        // Add more cases for other pattern types as needed
    }

    enemy_attacking = ENEMY_NONE;
    show_or_hide_enemy_combat_interface(false);
}

//// Pattern-specific functions
// Check -> Determine if the enemy can launch the pattern
// Launch -> Initiate the pattern
// Do -> Apply ongoing pattern effects
// Finish -> Conclude the pattern and apply final effects

// Electric pattern: CHECK
void check_electric_pattern(u8 numenemy, u8 npattern) {
    if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
        if (pattern_effect_in_progress == PTRN_HIDE) {
            obj_enemy[numenemy].last_pattern_time[npattern] -= 50; // Delay attack if player is hidden
        } else {
            enemy_launch_pattern(numenemy, npattern);
        }
    } else {
        if ((random() % 2) == 0) obj_enemy[numenemy].last_pattern_time[npattern]++; // 50% chance to progress cooldown
    }
}

// Electric pattern: LAUNCH
void launch_electric_pattern(void) {
    play_pattern_sound(PTRN_ELECTRIC); // Play thunder sound
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

// Electric pattern: DO
void do_electric_pattern_effect(void) {
    if (frame_counter % 2 == 0) VDP_setHilightShadow(true); // Create flickering thunder effect
    else VDP_setHilightShadow(false);

    // If player launches a reversed thunder spell, end the effect early
    if (pattern_effect_in_progress == PTRN_ELECTRIC && pattern_effect_reversed == true) {
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC) - 1;
    }
}

// Electric pattern: FINISH
void finish_electric_pattern_effect(void) {
    VDP_setHilightShadow(false);
    if (pattern_effect_reversed == true && pattern_effect_in_progress == PTRN_ELECTRIC) {
        hit_enemy(enemy_attacking); // Player successfully countered, damage the enemy
        pattern_effect_in_progress = PTRN_NONE;
        pattern_effect_reversed = false;
    } else {
        hit_caracter(active_character); // Player failed to counter, take damage
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][2]);
        talk_dialog(&dialogs[ACT1_DIALOG3][3]);
        show_or_hide_interface(true);
        show_or_hide_enemy_combat_interface(true);
    }
}

// Bite pattern: CHECK
void check_bite_pattern(u8 numenemy, u8 npattern) {
    if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
        if (pattern_effect_in_progress == PTRN_HIDE) {
            obj_enemy[numenemy].last_pattern_time[npattern] -= 50; // Delay attack if player is hidden
        } else {
            enemy_launch_pattern(numenemy, npattern);
        }
    } else {
        if ((random() % 2) == 0) obj_enemy[numenemy].last_pattern_time[npattern]++; // 50% chance to progress cooldown
    }
}

// Bite pattern: LAUNCH
void launch_bite_pattern(void) {
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

// Bite pattern: DO
void do_bite_pattern_effect(void) {
    // If player is hidden, end the effect early
    if (pattern_effect_in_progress == PTRN_HIDE) {
        enemy_attack_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE) - 1;
    }
}

// Bite pattern: FINISH
void finish_bite_pattern_effect(void) {
    if (pattern_effect_in_progress == PTRN_HIDE) {
        // Do nothing
    } else {
        show_character(active_character,true);
        hit_caracter(active_character); // Player failed to avoid, take damage
        show_or_hide_interface(false);
        show_or_hide_enemy_combat_interface(false);
        talk_dialog(&dialogs[ACT1_DIALOG3][2]);
        talk_dialog(&dialogs[ACT1_DIALOG3][4]);
        show_or_hide_interface(true);
        show_or_hide_enemy_combat_interface(true);
        show_character(active_character,true);
    }
}