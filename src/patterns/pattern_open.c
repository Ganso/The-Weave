#include "globals.h"

// Launch callback — currently does nothing
void playerOpenLaunch(void)
{
}

// Update callback
bool playerOpenUpdate(void)
{
    const u16 duration = playerPatterns[PATTERN_OPEN].baseDuration;

    if (combatContext.effectTimer >= duration)
        return true;        // finished
    return false;           // keep running
}

bool playerOpenCanUse(void)
{
    return false;           // never available
}