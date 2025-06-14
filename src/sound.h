#ifndef _SOUNDS_H_
#define _SOUNDS_H_

#define XGM_VERSION 2 // XGM or XGM2 sound driver

// VGM and WAV resources should be defined as XGM or XGM2 according to the driver version

void play_music(const u8 *music); // Start background music
void fade_music(u16 numframes); // Fade out the music over N frames
void play_sample(const u8 *sample, u32 len); // Play a sound sample

// Notes and patterns sound effects
void play_player_note(u8 nnote); // Play a note (player)
void play_enemy_note(u8 nnote); // Play a note (enemy)
void play_player_pattern_sound(u16 npattern); // Play the sound of a pattern spell (player)
void play_enemy_pattern_sound(u16 npattern); // Play the sound of a pattern spell (enemy)


#endif
