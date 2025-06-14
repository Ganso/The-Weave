#ifndef PATTERN_OPEN_H
#define PATTERN_OPEN_H

#include "globals.h"

// Launch the spell
void player_open_launch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool player_open_update(void);

// Check if the pattern can be used
bool player_open_can_use(void);

#endif // PATTERN_OPEN_H
