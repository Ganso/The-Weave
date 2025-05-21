#ifndef PATTERN_OPEN_H
#define PATTERN_OPEN_H

#include "globals.h"

// Launch the spell
void playerOpenLaunch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool playerOpenUpdate(void);

// Check if the pattern can be used
bool playerOpenCanUse(void);

#endif // PATTERN_OPEN_H
