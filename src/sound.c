#include <genesis.h>
#include "globals.h"

void play_music(const u8 *music)
{
    if (XGM_VERSION==2) XGM2_play(music);
    else XGM_startPlay(music);
}

void fade_music(u16 numframes)
{
    if (XGM_VERSION==2) XGM2_fadeOutAndStop(numframes);
    else {} // Fade out not supported in XGM1
}

void play_sample(const u8 *sample, u32 len)
{
    if (XGM_VERSION==2) XGM2_playPCM(sample, len, SOUND_PCM_CH_AUTO);
    else {
        XGM_setPCM(1, sample, len);
        XGM_startPlayPCM(1, 1, SOUND_PCM_CH2);
    }
}
