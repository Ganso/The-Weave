#include "globals.h"

// Launch callback — currently does nothing
void enemyThunderLaunch(u8 enemyId)
{
    (void)enemyId;    // unused for now
}

// Update callback — returns true when finished
bool enemyThunderUpdate(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration;

    // PENDING: Flash screen

    // Freeze the effect if the player is playing their own thunder pattern
    if (combat_state == COMBAT_STATE_PLAYER_PLAYING)
        return false;

    // Advance the effect timer only if in ENEMY_EFFECT state
    combatContext.effectTimer++;

    // Upon expiration, if NOT countered, deal damage
    if (combatContext.effectTimer >= duration)
    {
        talk_dialog(&dialogs[ACT1_DIALOG3][1]);
        hit_player(1); // -1 HP to the player
        // PENDING: Stop flashing
        return true; // Finished
    }

    return false; // Still running
}

// Called when the pattern is countered — currently does nothing
void enemyThunderOnCounter(u8 enemyId)
{
    hit_enemy(enemyId, 1);               // -1 HP
    talk_dialog(&dialogs[ACT1_DIALOG3][2]);
    enemyPatterns[enemyId][0].rechargeFrames = SCREEN_FPS * 3;
}