#ifndef ENEMY_ELECTRIC_PATTERN_H
#define ENEMY_ELECTRIC_PATTERN_H

#include "../globals.h"

// Forward declarations
struct StateMachine;

// External variables needed
extern u16 enemy_attacking;
extern bool enemy_attack_effect_in_progress;
extern bool counter_spell_success;
// ENEMY_NONE is a macro defined in enemies.h

// Enemy electric pattern functions
void enemy_electric_pattern_launch(struct StateMachine* sm);
void enemy_electric_pattern_do(struct StateMachine* sm);
void enemy_electric_pattern_finish(struct StateMachine* sm);

#endif // ENEMY_ELECTRIC_PATTERN_H