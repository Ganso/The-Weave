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
    const u8 *sample = NULL;

    switch (noteCode)
    {
        case NOTE_MI:  sample = snd_note_mi;  break;
        case NOTE_FA:  sample = snd_note_fa;  break;
        case NOTE_SOL: sample = snd_note_sol; break;
        case NOTE_LA:  sample = snd_note_la;  break;
        case NOTE_SI:  sample = snd_note_si;  break;
        case NOTE_DO:  sample = snd_note_do;  break;
        default: return;
    }

    play_music(sample);
}

// ------------------------------------------------------------------
// Enemy note
// ------------------------------------------------------------------
void playEnemyNote(u8 noteCode)
{
    const u8 *sample = NULL;

    switch (noteCode)
    {
        case NOTE_MI:  sample = snd_enemy_note_mi;  break;
        case NOTE_FA:  sample = snd_enemy_note_fa;  break;
        case NOTE_SOL: sample = snd_enemy_note_sol; break;
        case NOTE_LA:  sample = snd_enemy_note_la;  break;
        case NOTE_SI:  sample = snd_enemy_note_si;  break;
        case NOTE_DO:  sample = snd_enemy_note_do;  break;
        default: return;
    }

    play_music(sample);
}


// ------------------------------------------------------------------
// Spell jingle (player side)
// ------------------------------------------------------------------
void playPlayerPatternSound(u16 patternId)
{
    switch (patternId)
    {
        case PATTERN_THUNDER:
            play_sample(snd_pattern_thunder, sizeof(snd_pattern_thunder));
            break;
        case PATTERN_HIDE:
            play_sample(snd_pattern_hide, sizeof(snd_pattern_hide));
            break;
        case PATTERN_OPEN:
            play_sample(snd_pattern_open, sizeof(snd_pattern_open));
            break;
        case PATTERN_SLEEP:
            // TODO
            break;           
        default:
            play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
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
            play_sample(snd_pattern_thunder, sizeof(snd_pattern_thunder));
            break;
        case PATTERN_EN_BITE:
            // TODO
            break;
        default:
            play_sample(snd_pattern_invalid, sizeof(snd_pattern_invalid));
            break;
    }
}
