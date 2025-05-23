#ifndef PATTERN_THUNDER_H
#define PATTERN_THUNDER_H

#include "globals.h"

// Launch the spell
void playerThunderLaunch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool playerThunderUpdate(void);

// Check if the pattern can be used
bool playerThunderCanUse(void);

#endif // PATTERN_THUNDER_H
