#ifndef _SOUNDS_H_
#define _SOUNDS_H_

#define XGM_VERSION 1 // XGM or XGM2 sound driver

// VGM and WAV resources should be defined as XGM or XGM2 according to the driver version

void play_music(const u8 *music);
void fade_music(u16 numframes);
void play_sample(const u8 *sample, u32 len);

#endif
