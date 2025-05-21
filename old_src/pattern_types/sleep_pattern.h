#ifndef SLEEP_PATTERN_H
#define SLEEP_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// Sleep pattern functions
bool sleep_pattern_can_use(void);
void sleep_pattern_launch(struct StateMachine* sm);
void sleep_pattern_do(struct StateMachine* sm);
void sleep_pattern_finish(struct StateMachine* sm);

#endif // SLEEP_PATTERN_H