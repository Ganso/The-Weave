#include <genesis.h>
#include "globals.h"

// Main state machine for enemy pattern system
void check_enemy_pattern_state(void)
{
    u8 numenemy, npattern;
    u16 max_effect_time;

    // Main state machine
    switch (obj_enemy[enemy_attacking].obj_character.state)
    {
        case STATE_IDLE:
            // Check for new patterns if no attack is in progress
            if (!enemy_attack_effect_in_progress && enemy_attacking == ENEMY_NONE) {
                for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
                    if (obj_enemy[numenemy].obj_character.active) {
                        for (npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                            if (obj_enemy[numenemy].class.has_pattern[npattern]) {
                                // Check if pattern is ready (cooldown)
                                if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
                                    // Don't attack if player is hidden
                                    if (pattern_effect_in_progress == PTRN_HIDE) {
                                        obj_enemy[numenemy].last_pattern_time[npattern] -= 50;
                                    } else {
                                        // Initialize attack
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
                                    // 50% chance to progress cooldown
                                    if ((random() % 2) == 0) {
                                        obj_enemy[numenemy].last_pattern_time[npattern]++;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            break;

        case STATE_PLAYING_NOTE:
            if (enemy_attack_time != calc_ticks(MAX_ATTACK_NOTE_PLAYING_TIME)) {
                enemy_attack_time++;
            } else {
                // Current note finished
                show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false, false);
                
                if (enemy_attack_pattern_notes != obj_Pattern_Enemy[enemy_attack_pattern].numnotes - 1) {
                    // Play next note
                    enemy_attack_pattern_notes++;
                    enemy_attack_time = 0;
                    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true, true);
                } else {
                    // Pattern complete, move to effect
                    obj_enemy[enemy_attacking].obj_character.state = STATE_PATTERN_EFFECT;
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
                    }
                }
                show_or_hide_enemy_combat_interface(true);
            }
            break;

        case STATE_PATTERN_EFFECT:
            // Determine max effect duration
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

            if (enemy_attack_effect_time < max_effect_time) {
                // Apply pattern-specific effect
                switch (enemy_attack_pattern) {
                    case PTRN_EN_ELECTIC:
                        do_electric_pattern_effect();
                        break;
                    case PTRN_EN_BITE:
                        do_bite_pattern_effect();
                        break;
                }
                enemy_attack_effect_time++;
            }

            if (enemy_attack_effect_time == max_effect_time) {
                if (num_played_notes != 0) {
                    // Player is mid-pattern, give them time to finish
                    enemy_attack_effect_time--;
                } else {
                    // Move to effect finish state
                    obj_enemy[enemy_attacking].obj_character.state = STATE_PATTERN_EFFECT_FINISH;
                }
            }
            break;

        case STATE_PATTERN_EFFECT_FINISH:
            finish_enemy_pattern_effect();
            obj_enemy[enemy_attacking].obj_character.state = STATE_IDLE;
            break;

        default:
            break;
    }
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
    }

    enemy_attacking = ENEMY_NONE;
    show_or_hide_enemy_combat_interface(false);
}

//// Pattern-specific effect functions

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
