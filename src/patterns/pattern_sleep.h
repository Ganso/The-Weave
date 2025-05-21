#ifndef PATTERN_SLEEP_H
#define PATTERN_SLEEP_H

#include "globals.h"

// Launch the spell
void playerSleepLaunch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool playerSleepUpdate(void);

// Check if the pattern can be used
bool playerSleepCanUse(void);

#endif // PATTERN_SLEEP_H
