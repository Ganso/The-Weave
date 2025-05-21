#ifndef PATTERN_HIDE_H
#define PATTERN_HIDE_H

#include "globals.h"

// Launch the spell
void playerHideLaunch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool playerHideUpdate(void);

// Check if the pattern can be used
bool playerHideCanUse(void);

#endif // PATTERN_HIDE_H
