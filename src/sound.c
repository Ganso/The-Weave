#include "globals.h"

void play_music(const u8 *music)    // Start playing background music track using XGM1/XGM2 driver
{
    if (XGM_VERSION==2) XGM2_play(music);
    else XGM_startPlay(music);
}

void fade_music(u16 numframes)    // Fade out current music over specified number of frames (XGM2 only)
{
    if (XGM_VERSION==2) XGM2_fadeOutAndStop(numframes);
    else {} // Fade out not supported in XGM1
}

void play_sample(const u8 *sample, u32 len)    // Play sound effect sample using available PCM channel
{
    if (XGM_VERSION==2) XGM2_playPCM(sample, len, SOUND_PCM_CH_AUTO);
    else {
        XGM_setPCM(1, sample, len);
        XGM_startPlayPCM(1, 1, SOUND_PCM_CH2);
    }
}


// ------------------------------------------------------------------
// Player note (simple pitch table)
// ------------------------------------------------------------------
void playPlayerNote(u8 noteCode)
{
    switch (noteCode)
    {
        case NOTE_MI:  /* PENDING - play sample */ break;
        case NOTE_FA:  /* PENDING */ break;
        case NOTE_SOL: /* PENDING */ break;
        case NOTE_LA:  /* PENDING */ break;
        case NOTE_SI:  /* PENDING */ break;
        case NOTE_DO:  /* PENDING */ break;
        default: break;
    }
}

// ------------------------------------------------------------------
// Enemy note (pitch slightly different / lower volume)
// ------------------------------------------------------------------
void playEnemyNote(u8 noteCode)
{
    // For now reuse the same samples
    playPlayerNote(noteCode);
}


// ------------------------------------------------------------------
// Spell jingle (player side)
// ------------------------------------------------------------------
void playPlayerPatternSound(u16 patternId)
{
    switch (patternId)
    {
        case PATTERN_THUNDER:
            // PENDING - playSample(snd_pattern_electric, sizeof(...));
            break;
        case PATTERN_HIDE:
            // PENDING
            break;
        case PATTERN_OPEN:
            // PENDING
            break;
        case PATTERN_SLEEP:
            // PENDING
            break;           
        default:
            // PENDING - play error jingle
            break;
    }
}

// ------------------------------------------------------------------
// Spell jingle (enemy side)
// ------------------------------------------------------------------
void playEnemyPatternSound(u16 patternId)
{
    switch (patternId)
    {
        case PATTERN_EN_THUNDER:
            // PENDING
            break;
        case PATTERN_EN_BITE:
            // PENDING
            break;
        default:
            // PENDING
            break;
    }
}
