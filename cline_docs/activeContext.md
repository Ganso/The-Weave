# Active Context

## Current Development Status
The game is in early stages of development, currently functioning as a technical demo targeting a late 2025 release.

## Recent Changes
- Implementation of Act 1 with multiple scenes
- Basic combat system with enemy encounters
- Musical pattern/spell system implementation
- Character dialogue system with choice mechanics
- Scene transitions and environment interactions

## Current Focus
- Act 1 implementation and polish
- Core gameplay mechanics refinement
- Technical demo development

## Next Steps
1. Complete remaining Act 1 scenes
2. Polish existing gameplay mechanics
3. Implement additional spells and patterns
4. Enhance combat system
    - Refactor Combat System (Propuesta 1: Sistema de Estados y Mensajes)
        - Phase 1: Preparation (Document current system, identify dependencies, create basic tests, backup code)
            - Dependencies:
                - `character_patterns.c`: `genesis.h`, `globals.h`
                - `enemies_patterns.c`: `genesis.h`, `globals.h`
                - `combat.c`: `genesis.h`, `globals.h`
                - `enemies.c`: `genesis.h`, `globals.h`
            - Global Variables:
                - `character_patterns.c`: `player_patterns_enabled`, `note_playing`, `note_playing_time`, `time_since_last_note`, `player_pattern_effect_in_progress`, `player_pattern_effect_reversed`, `player_pattern_effect_time`, `played_notes[4]`, `num_played_notes`, `obj_pattern[MAX_PATTERNS]`
                - `enemies_patterns.c`: `obj_Pattern_Enemy[MAX_PATTERN_ENEMY]`, `enemy_attacking`, `enemy_attack_pattern`, `enemy_attack_pattern_notes`, `enemy_attack_time`, `enemy_attack_effect_in_progress`, `enemy_attack_effect_time`, `enemy_note_active[6]`
                - `combat.c`: `is_combat_active`
                - `enemies.c`: `obj_enemy[MAX_ENEMIES]`, `spr_enemy[MAX_ENEMIES]`, `spr_enemy_face[MAX_ENEMIES]`, `spr_enemy_shadow[MAX_ENEMIES]`, `obj_enemy_class[MAX_ENEMY_CLASSES]`
        - Phase 2: Refactorización Base
            - Create `PatternState` enum in `combat.h`: `STATE_IDLE`, `STATE_PLAYING_NOTE`, `STATE_PATTERN_CHECK`, `STATE_PATTERN_EFFECT`, `STATE_PATTERN_EFFECT_FINISH`, `STATE_ATTACK_FINISHED`
            - Create `PatternStateManager` struct in `combat.h`: `current_state`, `timer`, `notes[4]`, `note_count`
            - Create `MessageType` enum in `combat.h`: `MSG_PATTERN_COMPLETE`, `MSG_COMBAT_START`, `MSG_ENEMY_DEFEATED`
            - Create `Message` struct in `combat.h`: `type`, `param`
            - Move global variables related to combat from `character_patterns.c`, `enemies_patterns.c`, `combat.c`, and `enemies.c` to the `PatternStateManager` struct.
            - Create header files for each of the files mentioned above if they don't exist and include them in the relevant files.
            - Use the new state machine library in the combat system.
        - Phase 3: Implementación Core
            - Modify `character_patterns.c`, `enemies_patterns.c`, and `combat.c` to use the `PatternStateManager` and the message system.
            - Modify `combat.c` to handle the different states of the combat system, using the new state machine library.
                - Implement a `combat_update` function that handles the state transitions based on messages.
                - Implement functions to send messages: `send_message(MessageType type, u16 param)`.
            - Modify `enemies.c` to react to combat messages.
        - Phase 4: Finalization
            - Comprehensive testing of all combat scenarios.
            - Ensure compatibility with SGDK.
            - Optimize performance.
            - Final documentation and code cleanup.
5. Add more character interactions and story elements
