# Combat System and Patterns Refactoring Proposal

## Index
1. [Current Situation](#current-situation)
2. [Objectives](#objectives)
3. [Architecture Proposals](#architecture-proposals)
4. [Solution Comparison](#solution-comparison)
5. [Implementation Plan](#implementation-plan)
6. [Conclusions](#conclusions)

## Current Situation

The current system presents the following challenges:

- Extensive use of global variables
- State logic scattered across multiple files
- High coupling between systems
- Difficulty in maintenance
- Complexity in tracking the execution flow
- Existence of a state machine in `entity.h` that needs to be replaced

Main files affected:
- character_patterns.c
- enemies_patterns.c
- combat.c
- enemies.c
- entity.h

## Objectives

1. **Main**
   - Reduce code complexity
   - Improve maintainability
   - Maintain performance
   - Facilitate debugging
   - Replace the existing state machine in `entity.h` with a new implementation in `statemachine.h`

2. **Technical**
   - Compatible with standard C
   - Compatible with SGDK
   - Efficient in memory usage
   - Optimized for Megadrive

## Architecture Proposals

### Proposal 1: State and Message System

```c
// State System
typedef enum {
    STATE_IDLE,
    STATE_CASTING,
    STATE_EFFECT,
    STATE_EFFECT_FINISH
} PatternState;

typedef struct {
    PatternState current_state;
    u16 timer;
    u8 notes[4];
    u8 note_count;
} PatternStateManager;

// Message System
typedef enum {
    MSG_PATTERN_COMPLETE,
    MSG_COMBAT_START,
    MSG_ENEMY_DEFEATED
} MessageType;

typedef struct {
    MessageType type;
    u16 param;
} Message;
```

#### Features
- Clearly defined states
- Lightweight message system
- Grouping of related variables
- Centralized state management

### Proposal 2: Subsystems and Commands

```c
// Subsystems
typedef struct {
    void (*init)(void);
    void (*update)(void);
    void (*cleanup)(void);
    bool (*can_execute_pattern)(u8 pattern_id);
    void (*start_pattern)(u8 pattern_id);
    void (*cancel_pattern)(void);
} PatternSystem;

// Command System
typedef enum {
    CMD_NONE,
    CMD_PLAY_NOTE,
    CMD_START_PATTERN,
    CMD_ATTACK_ENEMY,
    CMD_END_TURN
} CommandType;

typedef struct {
    CommandType type;
    u16 target;
    u8 param;
} Command;
```

#### Features
- Independent modules
- Command system for actions
- Centralized resource manager
- Data organized by subsystem

## Solution Comparison

| Characteristic | Current System | Proposal 1 | Proposal 2 |
|---------------|----------------|-------------|-------------|
| Complexity | High | Medium | Medium |
| Maintainability | Low | High | High |
| Memory Usage | Medium | Low | Medium |
| Performance | High | High | High |
| Debugging Ease | Low | Medium | High |
| Scalability | Low | Medium | High |
| Coupling | High | Low | Low |

### Advantages and Disadvantages

#### Proposal 1: States and Messages
Pros:
- Simpler to implement
- Lower memory usage
- Smoother transition from current system

Cons:
- Less flexible than Proposal 2
- Limited to predefined patterns
- Lower extensibility

#### Proposal 2: Subsystems and Commands
Pros:
- More flexible and extensible
- Better code organization
- Easier to debug

Cons:
- More complex implementation
- Higher memory usage
- Steeper learning curve

## Implementation Plan

### Phase 1: Preparation
1. Document current system
2. Identify dependencies
3. Create basic tests
4. Backup the code

### Phase 2: Base Refactoring
1. Group variables into structures
2. Centralize definitions
3. Create header files
4. Document interfaces

### Phase 3: Core Implementation
1. Implement chosen system
2. Migrate functionality gradually
3. Verify operation
4. Optimize performance

### Phase 4: Finalization
1. Complete testing
2. Final documentation
3. Code cleanup
4. Performance verification

### Example Timeline

| Week | Task | Description |
|--------|-------|-------------|
| 1 | Preparation | Documentation and analysis |
| 2 | Base Structures | Definition of types and structures |
| 3-4 | Core System | Main implementation |
| 5 | Migration | Transition of existing code |
| 6 | Testing | Testing and optimization |

## Conclusions

### Recommendation
It is recommended to implement **Proposal 1** initially, with the possibility of evolving to Proposal 2 in the future:

1. Lower initial impact
2. Faster to implement
3. Maintains performance
4. Solid base for future improvements

### Expected Benefits
- More maintainable code
- Better organization
- Easier to extend
- Better long-term performance

### Next Steps
1. Refactor Combat System (Proposal 1: State and Message System)
    - Phase 1: Preparation (Document current system, identify dependencies, create basic tests, backup code)
        - Dependencies:
            - `character_patterns.c`: `genesis.h`, `globals.h`
            - `enemies_patterns.c`: `genesis.h`, `globals.h`
            - `combat.c`: `genesis.h`, `globals.h`
            - `enemies.c`: `genesis.h`, `globals.h`
            - `entity.h`: (Existing state machine)
            - `statemachine.h`: (New state machine)
        - Global Variables:
            - `character_patterns.c`: `player_patterns_enabled`, `note_playing`, `note_playing_time`, `time_since_last_note`, `player_pattern_effect_in_progress`, `player_pattern_effect_reversed`, `player_pattern_effect_time`, `played_notes[4]`, `num_played_notes`, `obj_pattern[MAX_PATTERNS]`
            - `enemies_patterns.c`: `obj_Pattern_Enemy[MAX_PATTERN_ENEMY]`, `enemy_attacking`, `enemy_attack_pattern`, `enemy_attack_pattern_notes`, `enemy_attack_time`, `enemy_attack_effect_in_progress`, `enemy_attack_effect_time`, `enemy_note_active[6]`
            - `combat.c`: `is_combat_active`
            - `enemies.c`: `obj_enemy[MAX_ENEMIES]`, `spr_enemy[MAX_ENEMIES]`, `spr_enemy_face[MAX_ENEMIES]`, `spr_enemy_shadow[MAX_ENEMIES]`, `obj_enemy_class[MAX_ENEMY_CLASSES]`
    - Phase 2: Gradual Transition to New State Machine
        - **Subphase 2.1: Decouple Combat-Related States from `entity.h`**
            - Step 1: Remove `STATE_PLAYING_NOTE`, `STATE_PATTERN_FINISHED`, `STATE_PATTERN_CHECK`, `STATE_PATTERN_EFFECT`, `STATE_PATTERN_EFFECT_FINISH`, `STATE_ATTACK_FINISHED` from the `GameState` enum in `entity.h`.
            - Step 2: Introduce a separate `CombatState` enum in `combat.h` with these combat-related states.
            - Step 3: Modify the `Entity` struct in `entity.h` to use the `CombatState` enum for combat-related state management.
        - **Subphase 2.2: Integrate `statemachine.h` and Initialize State Machine**
            - Step 1: Include `statemachine.h` in `combat.c`.
            - Step 2: Create a `StateMachine` instance within the combat system.
            - Step 3: Call `StateMachine_Init` to initialize the state machine.
        - **Subphase 2.3: Migrate State Logic - Initial State**
            - Step 1: Set the initial state of the combat system to `SM_STATE_IDLE`.
            - Step 2: Ensure the combat system functions correctly in the `SM_STATE_IDLE` state.
        - **Subphase 2.4: Migrate State Logic - Pattern Playing**
            - Step 1: Migrate the logic for playing patterns to the `StateMachine_Update` function when the state is `SM_STATE_CASTING`.
            - Step 2: Ensure the combat system functions correctly when playing patterns.
        - **Subphase 2.5: Migrate State Logic - Effect and Finish**
            - Step 1: Migrate the logic for applying effects and finishing patterns to the `StateMachine_Update` function when the state is `SM_STATE_EFFECT` or `SM_STATE_EFFECT_FINISH`.
            - Step 2: Ensure the combat system functions correctly when applying effects and finishing patterns.
        - **Subphase 2.6: Finalize State Machine Transition**
            - Step 1: Remove any remaining dependencies on the old state machine in `entity.h` for combat-related logic.
            - Step 2: Ensure that all combat-related state transitions are handled by the new state machine in `statemachine.h`.
    - Phase 3: Refactorización Base
        - Create `PatternState` enum in `combat.h`: `STATE_IDLE`, `STATE_PLAYING_NOTE`, `STATE_PATTERN_CHECK`, `STATE_PATTERN_EFFECT`, `STATE_PATTERN_EFFECT_FINISH`, `STATE_ATTACK_FINISHED`
        - Create `PatternStateManager` struct in `combat.h`: `current_state`, `timer`, `notes[4]`, `note_count`
        - Create `MessageType` enum in `combat.h`: `MSG_PATTERN_COMPLETE`, `MSG_COMBAT_START`, `MSG_ENEMY_DEFEATED`
        - Create `Message` struct in `combat.h`: `type`, `param`
        - Move global variables related to combat from `character_patterns.c`, `enemies_patterns.c`, `combat.c`, and `enemies.c` to the `PatternStateManager` struct.
    - Phase 4: Implementación Core
        - Modify `character_patterns.c`, `enemies_patterns.c`, and `combat.c` to use the `PatternStateManager` and the message system.
        - Modify `combat.c` to handle the different states of the combat system.
            - Implement a `combat_update` function that handles the state transitions based on messages.
            - Implement functions to send messages: `send_message(MessageType type, u16 param)`.
        - Modify `enemies.c` to react to combat messages.
    - Phase 5: Finalization
        - Comprehensive testing of all combat scenarios.
        - Ensure compatibility with SGDK.
        - Optimize performance.
        - Final documentation and code cleanup.

### Final Considerations
- Maintain compatibility with SGDK
- Prioritize performance
- Document changes
- Perform continuous testing
- Implement the changes gradually, ensuring that the code remains operational between phases
