#include "globals.h"

// Launch callback — currently does nothing
void enemy_bite_launch(u8 enemyId)
{
    (void)enemyId;    // unused for now
}

// Update callback — returns true when finished
bool enemy_bite_update(u8 enemyId)
{
    const u16 duration = enemyPatterns[enemyId][0].baseDuration;

    if (combatContext.effectTimer >= duration)
        return true;     // finished
    return false;        // keep running
}
