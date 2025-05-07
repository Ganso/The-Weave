#ifndef COUNTER_SPELL_H
#define COUNTER_SPELL_H

#include "globals.h"

// Forward declarations
struct StateMachine;

// Counter-spell system functions
bool can_counter_spell(u16 player_pattern, u16 enemy_pattern, bool is_reverse);
void reset_all_states_after_counter_spell(u16 enemy_id);
void execute_counter_spell(struct StateMachine* player_sm, struct StateMachine* enemy_sm, u16 pattern_id);
void handle_counter_spell_result(bool success, u16 enemy_id, u16 pattern_id);

// Global flags and variables
extern bool counter_spell_success;
extern u16 pending_counter_hit_enemy;
extern u16 player_pattern_effect_in_progress;
extern bool player_pattern_effect_reversed;

#endif // COUNTER_SPELL_H