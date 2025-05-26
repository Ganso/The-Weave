#include "globals.h"

// Launch callback — currently does nothing
void enemyThunderLaunch(u8 enemyId)
{
    (void)enemyId;    // unused for now
}

// Update callback — returns true when finished
bool enemyThunderUpdate(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration; // slot 0 = Electric/Thunder

    if (combatContext.effectTimer >= duration)
        return true;     // finished
    return false;        // keep running
}

// Called when the pattern is countered — currently does nothing
void enemyThunderOnCounter(u8 enemyId)
{
    (void)enemyId;
}
