#ifndef _PATTERN_HIDE_H_
#define _PATTERN_HIDE_H_

#include "globals.h"

// Launch the spell
void player_hide_launch(void);

// Update the ongoing spell.
// Returns true when the effect is finished.
bool player_hide_update(void);

// Check if the pattern can be used
bool player_hide_can_use(void);

#endif // _PATTERN_HIDE_H_
