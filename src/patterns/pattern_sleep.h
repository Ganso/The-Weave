#ifndef _PATTERN_SLEEP_H_
#define _PATTERN_SLEEP_H_

#include "globals.h"

// Launch the spell
void player_sleep_launch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool player_sleep_update(void);

// Check if the pattern can be used
bool player_sleep_can_use(void);

#endif // _PATTERN_SLEEP_H_
