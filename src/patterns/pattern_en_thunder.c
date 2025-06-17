#include "globals.h"

// Local data
static u16  savedColor;     // Palette entry to restore after flash
static bool flashOn;        // TRUE = CRAM entry is white, FALSE = original

// Macro helpers
#define PAL_ENTRY(pal,col)  (((pal)<<4)|(col))
#define PAL0_COL4           PAL_ENTRY(0,4)
#define COLOR_ENEMY_FLASH     RGB24_TO_VDPCOLOR(0x4444FF)

// Launch callback — currently does nothing
void enemy_thunder_launch(u8 enemyId)
{
    savedColor = PAL_getColor(PAL0_COL4);
    flashOn    = false;
    SPR_setAnim(spr_enemy[enemyId], ANIM_ACTION);   // playing
}

// Update callback — returns true when finished
bool enemy_thunder_update(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration;

    // Flash phase only (notes handled by engine)
    if (!flashOn) {                           // first frame of flash
        PAL_setColor(PAL0_COL4, COLOR_ENEMY_FLASH);
        play_enemy_pattern_sound(PATTERN_EN_THUNDER); // play thunder sound
        flashOn = true;
    } else if ((frame_counter & 1) == 0) {    // toggle every 2 frames
        u16 col = PAL_getColor(PAL0_COL4);
        PAL_setColor(PAL0_COL4, (col == COLOR_ENEMY_FLASH) ? savedColor
                                                         : COLOR_ENEMY_FLASH);
    }

    if (combatContext.playerNotes > 0 && combatContext.playerNotes < 4)
        return false;                         // freeze while player typing

    if (++combatContext.effectTimer >= duration)
    {
        PAL_setColor(PAL0_COL4, savedColor);  // restore sky
        SPR_setAnim(spr_enemy[enemyId], ANIM_IDLE);
        if (obj_character[active_character].state != STATE_HIT)
        {
            hit_player(1);                  // -1 HP + enter STATE_HIT
        }
        return true;                          // effect finished
    }
    return false;
}



// Called when the pattern is countered
void enemy_thunder_on_counter(u8 enemyId)
{
    dprintf(2, "Enemy %d: Thunder pattern countered", enemyId);

    // Stop flashing immediately
    PAL_setColor(PAL0_COL4, savedColor);
    flashOn = false;

    // Apply damage to the enemy
    hit_enemy(enemyId, 1);

    // Cool-down so the ghost cannot spam thunder
    enemyPatterns[enemyId][0].rechargeFrames=enemyPatterns[enemyId][0].baseDuration; // Reset cooldown

    cancel_enemy_pattern(enemyId); // Reset enemy state
    cancel_player_pattern(); // Reset player state
}

// ---------------------------------------------------------------------------
// Cancel thunder without hitting the enemy (used by the HIDE spell)
// ---------------------------------------------------------------------------
void enemy_thunder_cancel(u8 enemyId)
{
    dprintf(2, "Enemy %d: Thunder cancelled by hide", enemyId);

    // Restore palette and stop flashing
    PAL_setColor(PAL0_COL4, savedColor);
    flashOn = false;

    // Cool-down so the enemy cannot spam thunder
    enemyPatterns[enemyId][0].rechargeFrames = enemyPatterns[enemyId][0].baseDuration;

    cancel_enemy_pattern(enemyId); // Reset enemy state
}
