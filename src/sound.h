#ifndef _SOUNDS_H_
#define _SOUNDS_H_

#define XGM_VERSION 2 // XGM or XGM2 sound driver

// VGM and WAV resources should be defined as XGM or XGM2 according to the driver version

void play_music(const u8 *music);
void fade_music(u16 numframes);
void play_sample(const u8 *sample, u32 len);

// Notes and patterns sound effects
void playPlayerNote(u8 nnote); // Play a note (player)
void playEnemyNote(u8 nnote); // Play a note (enemy)
void playPlayerPatternSound(u16 npattern); // Play the sound of a pattern spell (player)
void playEnemyPatternSound(u16 npattern); // Play the sound of a pattern spell (enemy)


#endif
