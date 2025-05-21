#include "patterns_registry.h"

// Registry storage
static Pattern registered_patterns[MAX_REGISTERED_PATTERNS];
static u16 pattern_count = 0;

void pattern_registry_init(void) {
    pattern_count = 0;
    // Initialize with empty patterns
    for (u16 i = 0; i < MAX_REGISTERED_PATTERNS; i++) {
        registered_patterns[i].id = PTRN_NONE;
        registered_patterns[i].active = false;
    }
}

void register_pattern(Pattern* pattern) {
    if (pattern_count < MAX_REGISTERED_PATTERNS) {
        registered_patterns[pattern_count] = *pattern;
        pattern_count++;
    }
}

Pattern* get_pattern(u16 pattern_id, u16 owner_type) {
    for (u16 i = 0; i < pattern_count; i++) {
        if (registered_patterns[i].id == pattern_id && 
            registered_patterns[i].owner_type == owner_type) {
            return &registered_patterns[i];
        }
    }
    return NULL;
}

u8 validate_pattern_sequence(u8* notes, bool* is_reverse, u16 owner_type) {
    u8 matches, reverse_matches;
    
    for (u16 i = 0; i < pattern_count; i++) {
        if (registered_patterns[i].owner_type != owner_type) {
            continue;
        }
        
        matches = 0;
        reverse_matches = 0;
        
        // Check forward match
        for (u8 j = 0; j < 4; j++) {
            if (notes[j] == registered_patterns[i].notes[j]) {
                matches++;
            }
            if (notes[j] == registered_patterns[i].notes[3-j]) {
                reverse_matches++;
            }
        }
        
        // Pattern found if all notes match and pattern is active
        if (matches == 4 && registered_patterns[i].active) {
            *is_reverse = false;
            return registered_patterns[i].id;
        }
        else if (reverse_matches == 4 && registered_patterns[i].active) {
            *is_reverse = true;
            return registered_patterns[i].id;
        }
    }
    
    return PTRN_NONE;
}
