#ifndef PATTERN_THUNDER_H
#define PATTERN_THUNDER_H

#include "globals.h"

// Launch the spell
void player_thunder_launch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool player_thunder_update(void);

// Check if the pattern can be used
bool player_thunder_can_use(void);

#endif // PATTERN_THUNDER_H
