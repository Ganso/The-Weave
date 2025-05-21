#ifndef BITE_PATTERN_H
#define BITE_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// External variables needed
extern u16 enemy_attacking;
extern bool enemy_attack_effect_in_progress;
// ENEMY_NONE is a macro defined in enemies.h

// Bite pattern functions
void bite_pattern_launch(struct StateMachine* sm);
void bite_pattern_do(struct StateMachine* sm);
void bite_pattern_finish(struct StateMachine* sm);

#endif // BITE_PATTERN_H