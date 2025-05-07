#ifndef OPEN_PATTERN_H
#define OPEN_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// Open pattern functions
bool open_pattern_can_use(void);
void open_pattern_launch(struct StateMachine* sm);
void open_pattern_do(struct StateMachine* sm);
void open_pattern_finish(struct StateMachine* sm);

#endif // OPEN_PATTERN_H