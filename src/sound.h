#ifndef _SOUNDS_H_
#define _SOUNDS_H_

#include "globals.h"

/**
 * @brief Play background music
 * Uses XGM/XGM2 driver based on XGM_VERSION
 * @param music Pointer to music data
 */
void play_music(const u8 *music);

/**
 * @brief Fade out current music
 * @param numframes Number of frames to fade over
 */
void fade_music(u16 numframes);

/**
 * @brief Play sound effect sample
 * @param sample Pointer to sample data
 * @param len Length of sample data
 */
void play_sample(const u8 *sample, u32 len);

#endif // _SOUNDS_H_
