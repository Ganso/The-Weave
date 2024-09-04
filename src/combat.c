#include <genesis.h>
#include "globals.h"

// Start (or end) a combat
void start_combat(bool start)
{
    u8 numenemy,npattern;

    if (start==true) { // Combat start
        setRandomSeed(frame_counter); // Initialize RNG
        is_combat_active=true;
        player_scroll_active=false; // Disable player scroll - Screen is fixed
        // Enemies
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) {
            if (obj_enemy[numenemy].obj_character.active==true) {
                obj_enemy[numenemy].hitpoints=obj_enemy[numenemy].class.max_hitpoints; // Initialize hitpoints
                for (npattern=0;npattern<MAX_PATTERN_ENEMY;npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern]==true) {
                        obj_enemy[numenemy].last_pattern_time[npattern]=random() % (obj_Pattern_Enemy[npattern].recharge_time/2); // Initialize pattern time
                    }
                }
            }
        enemy_attacking=ENEMY_NONE;
        enemy_attack_pattern_notes=0;
        enemy_attack_time=0;
        attack_effect_in_progress=0;
        }

    }

    else { // Combat end
        is_combat_active=false;
        player_scroll_active=true;
    }
}

// Enemy lauches a pattern
void enemy_launch_pattern(u8 numenemy, u8 npattern)
{
    if (enemy_attacking==ENEMY_NONE) { // No other enemy is attacking
        enemy_attack_pattern_notes=0;
        enemy_attack_time=0;
        enemy_attack_pattern=npattern;
        enemy_attacking=numenemy;
        anim_enemy(numenemy,ANIM_ACTION);
        enemy_launch_pattern_note();
    }
}

// Enemy launches a pattern note
void enemy_launch_pattern_note()
{
    show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true);
}

void show_enemy_note(u8 nnote, bool visible)
{
    Sprite **rodsprite;
    SpriteDefinition *rodspritedef;
    u8 *notesong;
    u16 rod_x;

    switch (nnote) 
    {
    case NOTE_MI:
        rodsprite = &spr_int_enemy_rod_1;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_1_sprite;
        notesong = (u8*)snd_note_mi;
        rod_x = 24;
        break;
    case NOTE_FA:
        rodsprite = &spr_int_enemy_rod_2;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_2_sprite;
        notesong = (u8*)snd_note_fa;
        rod_x = 24 + 32;
        break;
    case NOTE_SOL:
        rodsprite = &spr_int_enemy_rod_3;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_3_sprite;
        notesong = (u8*)snd_note_sol;
        rod_x = 24 + 64;
        break;
    case NOTE_LA:
        rodsprite = &spr_int_enemy_rod_4;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_4_sprite;
        notesong = (u8*)snd_note_la;
        rod_x = 24 + 96;
        break;
    case NOTE_SI:
        rodsprite = &spr_int_enemy_rod_5;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_5_sprite;
        notesong = (u8*)snd_note_si;
        rod_x = 24 + 128;
        break;
    default:
        rodsprite = &spr_int_enemy_rod_6;
        rodspritedef = (SpriteDefinition*) &int_enemy_rod_6_sprite;
        notesong = (u8*)snd_note_do;
        rod_x = 24 + 160;
        break;
    }

    if (visible == true) {
        *rodsprite = SPR_addSpriteSafe(rodspritedef, rod_x, 184, TILE_ATTR(PAL2, false, false, false));
        XGM_setLoopNumber(0);
        XGM_startPlay(notesong);
    }
    else {
        if (*rodsprite != NULL) {
            SPR_releaseSprite(*rodsprite);
            *rodsprite = NULL;
        }
    }
    SPR_update();
}

// Hit an enemy
void hit_enemy(u16 nenemy)
{
    u16 n, alive_enemies=0;

    obj_enemy[nenemy].hitpoints--;
    if (obj_enemy[nenemy].hitpoints==0) { // Enemy has no more hitpoints left
        SPR_setVisibility(spr_int_life_counter, HIDDEN); // Hide life counter
        release_enemy(nenemy); // Kill him
        for (n=0;n<MAX_ENEMIES;n++) {
            if (obj_enemy[n].obj_character.active==true) {
                alive_enemies++; // Count how many enemies are still alive
            }
        }
        if (alive_enemies==0) { // Everyone's dead
            start_combat(false);
        }
    }
    else {
        SPR_setAnim(spr_int_life_counter, obj_enemy[enemy_attacking].hitpoints-1); // Update life counter
        SPR_update();
    }
}

// Hit a character
void hit_caracter(u16 nchar)
{
    XGM2_playPCM(snd_player_hurt,sizeof(snd_player_hurt),SOUND_PCM_CH_AUTO);
}

// Check if an enemy is going to launch a pattern
void check_enemy_pattern(void)
{
    u8 numenemy, npattern;

    // Only check for new patterns if no attack is in progress
    if (attack_effect_in_progress == false && enemy_attacking == ENEMY_NONE) {
        for (numenemy = 0; numenemy < MAX_ENEMIES; numenemy++) {
            if (obj_enemy[numenemy].obj_character.active == true) {
                for (npattern = 0; npattern < MAX_PATTERN_ENEMY; npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern] == true) {
                        // Check pattern-specific conditions
                        switch (npattern) {
                            case PTRN_EN_ELECTIC:
                                check_electric_pattern(numenemy, npattern);
                                break;
                            case PTRN_EN_BITE:
                                check_bite_pattern(numenemy, npattern);
                                break;
                            // Add more cases for other pattern types
                        }
                    }
                }
            }
        }
    }

    // Handle ongoing attack
    if (attack_effect_in_progress == false && enemy_attacking != ENEMY_NONE) {
        // Show attacking enemy face (and no one else) and life counter
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
        SPR_setVisibility(spr_enemy_face[enemy_attacking], VISIBLE);
        SPR_setVisibility(spr_int_life_counter, VISIBLE);
        SPR_setAnim(spr_int_life_counter, obj_enemy[enemy_attacking].hitpoints-1);
        // Do the attack
        handle_ongoing_attack();
    }

    // Handle attack effect
    if (attack_effect_in_progress == true) {
        do_enemy_pattern_effect();
    }
}

// Handle ongoing attack
void handle_ongoing_attack(void)
{
    if (enemy_attack_time != MAX_ATTACK_NOTE_PLAYING_TIME) {
        enemy_attack_time++;
    } else {
        // Note finished
        show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false);
        
        if (enemy_attack_pattern_notes != obj_Pattern_Enemy[enemy_attack_pattern].numnotes - 1) {
            // Play next note
            enemy_attack_pattern_notes++;
            enemy_attack_time = 0;
            show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true);
        } else {
            // Notes finished, start pattern effect
            attack_effect_in_progress = true;
            attack_effect_time = 0;
            
            // Launch pattern-specific effect
            switch (enemy_attack_pattern) {
                case PTRN_EN_ELECTIC:
                    launch_electric_pattern();
                    break;
                case PTRN_EN_BITE:
                    launch_bite_pattern();
                    break;
                // Add more cases for other pattern types
            }
        }
    }
}

// An enemy pattern is in progress. Make something happen.
void do_enemy_pattern_effect(void) {
    u16 max_effect_time;

    // Determine max effect time based on pattern
    switch (enemy_attack_pattern) {
        case PTRN_EN_ELECTIC:
            max_effect_time = MAX_EFFECT_TIME_ELECTRIC;
            break;
        case PTRN_EN_BITE:
            max_effect_time = MAX_EFFECT_TIME_BITE;
            break;
        default:
            max_effect_time = 100; // Default value
    }

    if (attack_effect_time < max_effect_time) {
        // Do pattern-specific effect
        switch (enemy_attack_pattern) {
            case PTRN_EN_ELECTIC:
                do_electric_pattern_effect();
                break;
            case PTRN_EN_BITE:
                do_bite_pattern_effect();
                break;
            // Add more cases for other pattern types
        }
        attack_effect_time++;
    }

    if (attack_effect_time == max_effect_time) {
        if (num_played_notes != 0) {
            // Player is playing a pattern. Give them time.
            attack_effect_time--;
        } else {
            // Finish attack
            finish_enemy_pattern_effect();
        }
    }    
}

// Finish the enemy pattern effect
void finish_enemy_pattern_effect(void) {
    anim_enemy(enemy_attacking, ANIM_IDLE);
    obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern] = 0;
    attack_effect_time = 0;
    attack_effect_in_progress = false;

    // Finish pattern-specific effect
    switch (enemy_attack_pattern) {
        case PTRN_EN_ELECTIC:
            finish_electric_pattern_effect();
            break;
        case PTRN_EN_BITE:
            finish_bite_pattern_effect();
            break;
        // Add more cases for other pattern types
    }

    enemy_attacking = ENEMY_NONE;
}




//// Pattern-specific functions
// Check -> Can the enemy launch it?
// Launch -> Do launch actions
// Do -> Do in-spell actions
// Finish -> Do finish actions

// Electric pattern

void check_electric_pattern(u8 numenemy, u8 npattern) {
    if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
        enemy_launch_pattern(numenemy, npattern);
    } else {
        if ((random() % 2) == 0) obj_enemy[numenemy].last_pattern_time[npattern]++;
    }
}

void launch_electric_pattern(void) {
    play_pattern_sound(PTRN_ELECTRIC); // Thunder sound
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

void do_electric_pattern_effect(void) {
    if (frame_counter % 2 == 0) VDP_setHilightShadow(true); // Thunder effect
    else VDP_setHilightShadow(false);

    if (pattern_effect_in_progress == PTRN_ELECTRIC && pattern_effect_reversed == true) { // If player lauches a reversed thunder spell
        attack_effect_time = MAX_EFFECT_TIME_ELECTRIC - 1; // Force effect to end on next frame
    }
}

void finish_electric_pattern_effect(void) {
    VDP_setHilightShadow(false);
    if (pattern_effect_reversed == true && pattern_effect_in_progress == PTRN_ELECTRIC) { // If player lauched a reversed thunder spell
        hit_enemy(enemy_attacking); // Hit the enemy
        pattern_effect_in_progress = PTRN_NONE;
        pattern_effect_reversed = false;
    } else {
        hit_caracter(active_character); // Otherwise, hit the player
    }
}

// Bite pattern

void check_bite_pattern(u8 numenemy, u8 npattern) {
    if (obj_enemy[numenemy].last_pattern_time[npattern] == obj_Pattern_Enemy[npattern].recharge_time) {
        if (pattern_effect_in_progress == PTRN_HIDE) { // Don't launch that spell is player is hidden
            obj_enemy[numenemy].last_pattern_time[npattern] -= 50;
        } else {
            enemy_launch_pattern(numenemy, npattern);
        }
    } else {
        if ((random() % 2) == 0) obj_enemy[numenemy].last_pattern_time[npattern]++;
    }
}

void launch_bite_pattern(void) {
    anim_enemy(enemy_attacking, ANIM_MAGIC);
}

void do_bite_pattern_effect(void) {
    if (pattern_effect_in_progress == PTRN_HIDE) { // If player lauches a hide thunder spell
        attack_effect_time = MAX_EFFECT_TIME_BITE - 1; // Force effect to end on next frame
    }
}

void finish_bite_pattern_effect(void) {
    if (pattern_effect_in_progress == PTRN_HIDE) { // If player is hidden
        hit_enemy(enemy_attacking); // Hit the enemy
    } else {
        hit_caracter(active_character); // Otherwise, hit the player
    }
}