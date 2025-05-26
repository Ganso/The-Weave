#include "globals.h"

// Launch callback — currently does nothing
void enemyBiteLaunch(u8 enemyId)
{
    (void)enemyId;    // unused for now
}

// Update callback — returns true when finished
bool enemyBiteUpdate(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration;

    if (combatContext.effectTimer >= duration)
        return true;     // finished
    return false;        // keep running
}
