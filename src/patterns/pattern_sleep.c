#include "globals.h"

// Launch callback — currently does nothing
void playerSleepLaunch(void)
{
}

// Update callback
bool playerSleepUpdate(void)
{
    const u16 duration = playerPatterns[PATTERN_HIDE].baseDuration;

    if (++combatContext.effectTimer >= duration)
        return true;        // finished
    return false;           // keep running
}

bool playerSleepCanUse(void)
{
    return false;           // never available
}