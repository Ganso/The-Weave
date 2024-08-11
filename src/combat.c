#include <genesis.h>
#include "globals.h"

// Start (or end) a combat
void start_combat(bool start)
{
    u8 numenemy,npattern;

    if (start==true) { // Combat start
        setRandomSeed(random_seed); // Initialize RNG
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

// Check if an enemy is going to launch a pattern
void check_enemy_pattern(void)
{
    u8 numenemy,npattern;

    if (attack_effect_in_progress==false && enemy_attacking==ENEMY_NONE) { // No one is attacking and no attack effect in progress
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) {
            if (obj_enemy[numenemy].obj_character.active==true) {
                for (npattern=0;npattern<MAX_PATTERN_ENEMY;npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern]==true) {
                        if (obj_enemy[numenemy].last_pattern_time[npattern]==obj_Pattern_Enemy[npattern].recharge_time) {
                            if (npattern==PTRN_EN_BITE && pattern_effect_in_progress==PTRN_HIDE) {
                                // You can't bite a hidden character
                                obj_enemy[numenemy].last_pattern_time[npattern]-=50;
                            }
                            else enemy_launch_pattern(numenemy,npattern);
                        }
                        else {
                            if ((random()%2)==0) obj_enemy[numenemy].last_pattern_time[npattern]++; // Inc a random number of times
                        }
                    }
                }
            }
        }
    }

    if (attack_effect_in_progress==false && enemy_attacking!=ENEMY_NONE) { // Attack in progress
        if (enemy_attack_time!=MAX_ATTACK_NOTE_PLAYING_TIME) {
            enemy_attack_time++;
        }
        else { // Note finished
            show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], false); // Stop current note
            if (enemy_attack_pattern_notes!=obj_Pattern_Enemy[enemy_attack_pattern].numnotes-1) { // The are still more notes to be played
                enemy_attack_pattern_notes++;
                enemy_attack_time=0;
                show_enemy_note(obj_Pattern_Enemy[enemy_attack_pattern].notes[enemy_attack_pattern_notes], true); // Play next note
            }
            else { // Notes finished
                attack_effect_in_progress=true; // Start pattern effect
                attack_effect_time=0;
            }
        }
    }

    if (attack_effect_in_progress==true) do_enemy_pattern_effect(); // Attack effect in progress
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
        rodspritedef = (SpriteDefinition*) &int_rod_1_sprite;
        notesong = (u8*)snd_note_mi;
        rod_x = 24;
        break;
    case NOTE_FA:
        rodsprite = &spr_int_enemy_rod_2;
        rodspritedef = (SpriteDefinition*) &int_rod_2_sprite;
        notesong = (u8*)snd_note_fa;
        rod_x = 24 + 32;
        break;
    case NOTE_SOL:
        rodsprite = &spr_int_enemy_rod_3;
        rodspritedef = (SpriteDefinition*) &int_rod_3_sprite;
        notesong = (u8*)snd_note_sol;
        rod_x = 24 + 64;
        break;
    case NOTE_LA:
        rodsprite = &spr_int_enemy_rod_4;
        rodspritedef = (SpriteDefinition*) &int_rod_4_sprite;
        notesong = (u8*)snd_note_la;
        rod_x = 24 + 96;
        break;
    case NOTE_SI:
        rodsprite = &spr_int_enemy_rod_5;
        rodspritedef = (SpriteDefinition*) &int_rod_5_sprite;
        notesong = (u8*)snd_note_si;
        rod_x = 24 + 128;
        break;
    default:
        rodsprite = &spr_int_enemy_rod_6;
        rodspritedef = (SpriteDefinition*) &int_rod_6_sprite;
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

// An ennemy pattern is in progress. Make something happern.
void do_enemy_pattern_effect(void) {
    u16 max_effect_time;
    max_effect_time=100;

    if (attack_effect_time==0) { // Effect starting
        anim_enemy(enemy_attacking,ANIM_MAGIC);
        if (enemy_attack_pattern==PTRN_EN_ELECTIC) play_pattern_sound(PTRN_ELECTIC); // Thunder sound
    }
    if (attack_effect_time<max_effect_time) {
        if (enemy_attack_pattern==PTRN_EN_ELECTIC) { // Thunder effect
            if (attack_effect_time%2==0) VDP_setHilightShadow(true);
            else VDP_setHilightShadow(false);
        }
        if (enemy_attack_pattern==PTRN_EN_BITE && pattern_effect_in_progress==PTRN_HIDE) { // Player is hidden. Bite does not have effect
            attack_effect_time=max_effect_time-1;
        }
        attack_effect_time++;
    }
    if (attack_effect_time==max_effect_time) {
        if (num_played_notes!=0) { // Player is playing a pattern. Give him time.
            attack_effect_time--;
        }
        else { // Finish attack
            anim_enemy(enemy_attacking,ANIM_IDLE);
            obj_enemy[enemy_attacking].last_pattern_time[enemy_attack_pattern]=0;
            attack_effect_time=0;
            attack_effect_in_progress=false;
            if (enemy_attack_pattern==PTRN_EN_ELECTIC) VDP_setHilightShadow(false);
            enemy_attacking=ENEMY_NONE;
        }
    }    
}
