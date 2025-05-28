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
   const u16 duration = enemyPatterns[enemyId][0].baseDuration;   // slot 0

    // Toggle flash every 2 frames
    if (frame_counter % 2 == 0)  
    {
        PAL_setColor(PAL0_COL4, flashOn ? savedColor : COLOR_WHITE_VDP);
        flashOn = !flashOn;
    }

    // (B) Freeze the timer if the player is currently entering a pattern
    if (combat_state == COMBAT_STATE_PLAYER_PLAYING) {
        dprintf(2, "Enemy %d: Thunder effect paused (player playing)", enemyId);
        return false;                   // Still active but not counting
    }

    // (C) Otherwise advance timer
    combatContext.effectTimer++;

    // (D) Timer expired → deal damage (unless already countered)
    if (combatContext.effectTimer >= duration)
    {
        // Restore palette
        PAL_setColor(PAL0_COL4, savedColor);
        flashOn = false;

        // Hurt the player (SFX + dialogue); you can replace hit_player()
        hit_player(1);                            // -1 HP or just SFX

        return true;      // Finished, state machine will switch back
    }

    return false;         // Effect still running
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