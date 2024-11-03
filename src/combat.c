#include <genesis.h>
#include "globals.h"

// Start or end a combat sequence
void start_combat(bool start)
{
    u8 numenemy,npattern,nnote;

    if (start==true) { // Combat start
        setRandomSeed(frame_counter); // Initialize RNG for unpredictable combat events
        is_combat_active=true;
        player_scroll_active=false; // Disable player scroll - Screen is fixed during combat

        // Initialize enemies for combat
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) {
            if (obj_enemy[numenemy].obj_character.active==true) {
                obj_enemy[numenemy].hitpoints=obj_enemy[numenemy].class.max_hitpoints; // Reset enemy HP to max
                for (npattern=0;npattern<MAX_PATTERN_ENEMY;npattern++) {
                    if (obj_enemy[numenemy].class.has_pattern[npattern]==true) {
                        // Randomize initial pattern cooldown to prevent all enemies attacking at once
                        obj_enemy[numenemy].last_pattern_time[npattern]=random() % (obj_Pattern_Enemy[npattern].recharge_time/2);
                    }
                }
            }
        }
        // Reset combat state variables
        enemy_attacking=ENEMY_NONE;
        enemy_attack_pattern_notes=0;
        enemy_attack_time=0;
        enemy_attack_effect_in_progress=0;
        for (nnote=0;nnote<6;nnote++) enemy_note_active[nnote]=false;

        // Initialize life counter sprite for enemy HP display
        if (spr_int_life_counter==NULL) spr_int_life_counter = SPR_addSprite (&int_life_counter_sprite, 164, 180, TILE_ATTR(PAL2, false, false, false));
        SPR_setVisibility(spr_int_life_counter, HIDDEN);
    }
    else { // Combat end
        is_combat_active=false;
        player_scroll_active=true; // Re-enable player movement
        if (spr_int_life_counter!=NULL) SPR_releaseSprite(spr_int_life_counter);
        spr_int_life_counter=NULL;
    }
}

// Handle damage to an enemy
void hit_enemy(u16 nenemy)
{
    u16 n, alive_enemies=0;

    XGM2_playPCM(snd_player_hit_enemy,sizeof(snd_player_hit_enemy),SOUND_PCM_CH_AUTO);

    obj_enemy[nenemy].hitpoints--;
    if (obj_enemy[nenemy].hitpoints==0) { // Enemy defeated
        SPR_setVisibility(spr_int_life_counter, HIDDEN); // Hide life counter
        release_enemy(nenemy); // Remove enemy from the game
        for (n=0;n<MAX_ENEMIES;n++) {
            if (obj_enemy[n].obj_character.active==true) {
                alive_enemies++; // Count remaining enemies
            }
        }
        if (alive_enemies==0) { // All enemies defeated, end combat
            start_combat(false);
        }
    }
    else {
        // Flash enemy life counter to indicate damage
        for (u8 nframes=0;nframes<SCREEN_FPS>>3;nframes++) {
            SPR_setVisibility(spr_int_life_counter,VISIBLE);
            SPR_update();
            SYS_doVBlankProcess();
            SPR_setVisibility(spr_int_life_counter,HIDDEN);
            SPR_update();
            SYS_doVBlankProcess();
        }
        SPR_setAnim(spr_int_life_counter, obj_enemy[enemy_attacking].hitpoints-1); // Update life counter display
        for (u8 nframes=0;nframes<SCREEN_FPS>>1;nframes++) { // Continue flashing for half a second
            SPR_setVisibility(spr_int_life_counter,VISIBLE);
            SPR_update();
            SYS_doVBlankProcess();
            SPR_setVisibility(spr_int_life_counter,HIDDEN);
            SPR_update();
            SYS_doVBlankProcess();
        }
    }
}

// Handle damage to a player character
void hit_caracter(u16 nchar)
{
    XGM2_playPCM(snd_player_hurt,sizeof(snd_player_hurt),SOUND_PCM_CH_AUTO);
    // Additional logic for character damage can be added here
}

// Display or hide enemy combat interface elements
void show_or_hide_enemy_combat_interface(bool show)
{
    u16 numenemy,nnote;

    if (show==true && interface_active==true && is_combat_active==true && enemy_attacking != ENEMY_NONE) {
        // Show attacking enemy face and life counter, hide others
        if (spr_enemy_face[enemy_attacking]!=NULL)  SPR_setVisibility(spr_enemy_face[enemy_attacking], VISIBLE);
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) {
            if (numenemy!=enemy_attacking && spr_enemy_face[numenemy]!=NULL) {
                SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
            }
        }
        if (spr_int_life_counter!=NULL) {
            SPR_setVisibility(spr_int_life_counter, VISIBLE);
            SPR_setAnim(spr_int_life_counter, obj_enemy[enemy_attacking].hitpoints-1);
        } 
        // Show active enemy notes
        for (nnote=0;nnote<6;nnote++) {
            if (enemy_note_active[nnote]) show_enemy_note(nnote+1, true, false);
            else show_enemy_note(nnote+1, false, false);
        }
    }
    else {
        // Hide all enemy faces, life counter, and notes
        for (numenemy=0;numenemy<MAX_ENEMIES;numenemy++) {
            if (spr_enemy_face[numenemy]!=NULL) SPR_setVisibility(spr_enemy_face[numenemy], HIDDEN);
        }
        if (spr_int_life_counter!=NULL) SPR_setVisibility(spr_int_life_counter, HIDDEN);
        for (nnote=0;nnote<6;nnote++) {
            show_enemy_note(nnote+1, false, false);
        }
    }
    SPR_update();
}