#include "globals.h"

// Launch callback — currently does nothing
void player_hide_launch(void)
{
}

// Update callback
bool player_hide_update(void)
{
    const u16 duration = playerPatterns[PATTERN_HIDE].baseDuration;

    if (combatContext.effectTimer >= duration)
        return true;        // finished
    return false;           // keep running
}

// Check if the pattern can be used
bool player_hide_can_use(void)
{
    if (combatContext.patternReversed) return false; // Not allowed in reverse mode
    return false; // Not available in normal mode anyway by now
}
