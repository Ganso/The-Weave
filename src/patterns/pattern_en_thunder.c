#include "globals.h"

// Local data
static u16  savedColor;     // Palette entry to restore after flash
static bool flashOn;        // TRUE = CRAM entry is white, FALSE = original

// Macro helpers
#define PAL_ENTRY(pal,col)  (((pal)<<4)|(col))
#define PAL0_COL4           PAL_ENTRY(0,4)
#define COLOR_WHITE_VDP     RGB24_TO_VDPCOLOR(0xFFFFFF)

// Launch callback — currently does nothing
void enemyThunderLaunch(u8 enemyId)
{
    // 1) Save current colour and start flash immediately
    savedColor = PAL_getColor(PAL0_COL4);
    PAL_setColor(PAL0_COL4, COLOR_WHITE_VDP);
    flashOn = true;

    // 2) Play the enemy's magic animation (re-using ANIM_MAGIC)
    SPR_setAnim(spr_enemy[enemyId], ANIM_MAGIC);

    // 3) Reset effect timer (also done by launchEnemyPattern, but safe)
    combatContext.effectTimer = 0;

    // 4) Play thunder sound
    play_sample(snd_pattern_thunder, sizeof(snd_pattern_thunder));
}

// Update callback — returns true when finished
bool enemyThunderUpdate(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration; // 1 s

    // toggle flash every 2 frames
    if ((frame_counter & 1) == 0)
    {
        PAL_setColor(PAL0_COL4, flashOn ? savedColor : COLOR_WHITE_VDP);
        flashOn = !flashOn;
    }

    // freeze while player mid-pattern
    if (combatContext.playerNotes > 0 && combatContext.playerNotes < 4)
        return false;

    // advance timer
    if (++combatContext.effectTimer >= duration)
    {
        PAL_setColor(PAL0_COL4, savedColor);
        flashOn = false;
        hit_player(1);
        return true;            // effect finished
    }
    return false;               // still flashing
}



// Called when the pattern is countered — currently does nothing
void enemyThunderOnCounter(u8 enemyId)
{
    // Stop flashing immediately
    PAL_setColor(PAL0_COL4, savedColor);
    flashOn = false;

    // Apply damage to the enemy
    hit_enemy(enemyId, 1);

    // Cool-down so the ghost cannot spam thunder
    enemyPatterns[enemyId][0].rechargeFrames=enemyPatterns[enemyId][0].baseDuration; // Reset cooldown
}