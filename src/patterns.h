#ifndef PATTERNS_H
#define PATTERNS_H

// Forward declarations
struct StateMachine;

// Pattern structure for spell patterns
typedef struct Pattern {
    u16 id;                     // Pattern identifier
    bool active;                // Whether pattern is available
    u8 notes[4];                // Sequence of notes for this pattern
    u16 owner_type;             // OWNER_PLAYER or OWNER_ENEMY
    u16 recharge_time;          // Time before pattern can be used again (for enemies)
    bool (*can_use)(void);      // Function to check if pattern can be used
    void (*launch)(struct StateMachine*);    // Function to launch pattern effect
    void (*do_effect)(struct StateMachine*); // Function to process ongoing effect
    void (*finish)(struct StateMachine*);    // Function to finish effect
    Sprite *sd;                 // Sprite data for pattern visualization
} Pattern;

#endif // PATTERNS_H