#ifndef PATTERNS_REGISTRY_H
#define PATTERNS_REGISTRY_H

#include "globals.h"

#define MAX_REGISTERED_PATTERNS 10
#define OWNER_PLAYER 0
#define OWNER_ENEMY 1

// Forward declarations
struct StateMachine;

// Pattern registry functions
void pattern_registry_init(void);
void register_pattern(Pattern* pattern);
Pattern* get_pattern(u16 pattern_id, u16 owner_type);
u8 validate_pattern_sequence(u8* notes, bool* is_reverse, u16 owner_type);

#endif // PATTERNS_REGISTRY_H