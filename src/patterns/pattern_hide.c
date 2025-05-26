#include "globals.h"

// Launch callback — currently does nothing
void playerHideLaunch(void)
{
}

// Update callback
bool playerHideUpdate(void)
{
    const u16 duration = playerPatterns[PATTERN_HIDE].baseDuration;

    if (combatContext.effectTimer >= duration)
        return true;        // finished
    return false;           // keep running
}

// C
bool playerHideCanUse(void)
{
    return false;           // never available
}