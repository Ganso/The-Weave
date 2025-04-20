# Combat State Integration Plan - Revised

## Current Status Analysis

### Entity GameState (entity.h)
Currently has a good set of core states:
- STATE_IDLE
- STATE_WALKING
- STATE_PLAYING_NOTE
- STATE_PATTERN_CHECK
- STATE_PATTERN_EFFECT
- STATE_PATTERN_EFFECT_FINISH
- STATE_ATTACK_FINISHED
- STATE_FOLLOWING

### Combat System (combat.c)
Currently handles:
- Combat initialization/cleanup
- Damage calculation
- UI management
- Uses is_combat_active flag

## Next Step: Context-Aware State Behavior

Instead of adding new combat states, we'll modify how existing states behave based on the combat context. This approach recognizes that entities maintain their fundamental states (idle, walking, etc.) regardless of combat status.

### 1. Enhance State Behavior System
```c
void update_entity_behavior(Entity* entity) {
    // Base behavior modified by combat context
    switch(entity->state) {
        case STATE_IDLE:
            if (is_combat_active) {
                // Combat-specific idle behavior
                // - Check for attack opportunities
                // - Monitor threat
                // - Consider defensive actions
            } else {
                // Normal idle behavior
                // - Standard animations
                // - Environmental interactions
            }
            break;
            
        case STATE_WALKING:
            if (is_combat_active) {
                // Combat movement
                // - Tactical positioning
                // - Maintain combat distance
                // - Dodge mechanics
            } else {
                // Normal walking
                // - Path following
                // - Exploration
            }
            break;
            
        // Other states follow similar pattern
    }
}
```

### 2. Update Combat System Integration
Modify combat.c to focus on context switching rather than state management:
```c
void start_combat(bool start) {
    if (start) {
        is_combat_active = true;
        // Initialize combat-specific systems
        // - Setup UI elements
        // - Initialize pattern tracking
        // - Set up threat assessment
    } else {
        is_combat_active = false;
        // Cleanup combat-specific systems
        // - Hide UI elements
        // - Reset pattern tracking
    }
    // Entities keep their current states but behavior will adapt
}
```

### 3. Pattern Integration
Enhance pattern system to be combat-aware:
```c
void handle_pattern(Entity* entity, Pattern* pattern) {
    switch(entity->state) {
        case STATE_PLAYING_NOTE:
            if (is_combat_active) {
                // Combat pattern mechanics
                // - Damage calculations
                // - Effect targeting
            } else {
                // Normal pattern effects
                // - Environmental interactions
                // - Puzzle solving
            }
            break;
            
        case STATE_PATTERN_EFFECT:
            if (is_combat_active) {
                // Combat effects
                // - Apply damage
                // - Status effects
            } else {
                // Normal effects
                // - World interactions
            }
            break;
    }
}
```

## Implementation Order

1. Modify entity update logic to check combat context
2. Update pattern system to handle combat/non-combat effects
3. Enhance combat.c to focus on context management
4. Add combat-specific behavior implementations
5. Test state behavior in both contexts

## Benefits

- Simpler, more intuitive state system
- More flexible entity behavior
- Clearer separation between state and context
- Easier to maintain and extend
- More natural gameplay transitions

## Considerations

- Ensure smooth transitions when combat starts/ends
- Keep combat UI synchronized with entity behavior
- Balance combat and non-combat behavior differences
- Consider performance of additional context checks

## Next Steps After This Implementation

1. Enhance pattern effects for combat
2. Improve enemy AI behavior in combat
3. Add more sophisticated combat mechanics
4. Implement status effects system