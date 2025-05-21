#ifndef ELECTRIC_PATTERN_H
#define ELECTRIC_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// Electric pattern functions
bool electric_pattern_can_use(void);
void electric_pattern_launch(struct StateMachine* sm);
void electric_pattern_do(struct StateMachine* sm);
void electric_pattern_finish(struct StateMachine* sm);

#endif // ELECTRIC_PATTERN_H