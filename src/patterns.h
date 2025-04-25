#ifndef PATTERNS_H
#define PATTERNS_H

// Pattern structure for spell patterns
typedef struct Pattern {
    bool active;
    u8 notes[4];
    void (*callback)(void);
    Sprite *sd;  // Sprite data for pattern visualization
} Pattern;

#endif