#include "globals.h"

// Local data
static bool flashOn;        // TRUE = CRAM entry is white, FALSE = original

// Macro helpers
#define PAL_ENTRY(pal,col)  (((pal)<<4)|(col))
#define PAL0_COL4           PAL_ENTRY(0,4)
#define COLOR_INITIAL_SKY     RGB24_TO_VDPCOLOR(0x1585c2) // Initial sky color
#define COLOR_ENEMY_FLASH     RGB24_TO_VDPCOLOR(0x4444a3) // Flash color for enemy thunder

// Launch callback — currently does nothing
void enemy_thunder_launch(u8 enemyId)
{
    dprintf(2, "Enemy %d: Thunder pattern launched", enemyId);
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
        PAL_setColor(PAL0_COL4, (col == COLOR_ENEMY_FLASH) ? COLOR_INITIAL_SKY
                                                         : COLOR_ENEMY_FLASH);
    }

    if (combatContext.playerNotes > 0 && combatContext.playerNotes < 4)
        return false;                         // freeze while player typing

    if (++combatContext.effectTimer >= duration)
    {
        PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);  // restore sky
        flashOn = false;                      // reset flash state
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
    PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);
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
    PAL_setColor(PAL0_COL4, COLOR_INITIAL_SKY);
    flashOn = false;

    // Cool-down so the enemy cannot spam thunder
    enemyPatterns[enemyId][0].rechargeFrames = enemyPatterns[enemyId][0].baseDuration;

    cancel_enemy_pattern(enemyId); // Reset enemy state
}
