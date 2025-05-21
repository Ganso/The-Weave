#include "globals.h"

// Launch callback — currently does nothing
void playerThunderLaunch(void)
{
}

// Update callback
bool playerThunderUpdate(void)
{
    const u16 duration = playerPatterns[PATTERN_ELECTRIC].baseDuration;

    if (++combatContext.effectTimer >= duration)
        return true;        // finished
    return false;           // keep running
}

bool playerThunderCanUse(void)
{
    return false;           // never available
}