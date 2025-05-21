#ifndef HIDE_PATTERN_H
#define HIDE_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// Hide pattern functions
bool hide_pattern_can_use(void);
void hide_pattern_launch(struct StateMachine* sm);
void hide_pattern_do(struct StateMachine* sm);
void hide_pattern_finish(struct StateMachine* sm);

#endif // HIDE_PATTERN_H