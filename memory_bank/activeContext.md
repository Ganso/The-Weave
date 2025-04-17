# Active Context

## Current Development Status
The game is in early stages of development, currently functioning as a technical demo targeting a late 2025 release.

## Recent Changes
- Implementation of Act 1 with multiple scenes
- Basic combat system with enemy encounters
- Musical pattern/spell system implementation
- Character dialogue system with choice mechanics
- Scene transitions and environment interactions
- Created `projectBrief.md` to document project requirements
- Initiated Memory Bank update

## Current Focus
- Act 1 implementation and polish
- Core gameplay mechanics refinement
- Technical demo development
- Updating Memory Bank with current project state
- Collision system optimization to improve character movement and interactions
  - ✅ Implemented professional sliding collision system in controller.c
  - ✅ Eliminated collision counter "hack" approach
  - ✅ Added response-based collision handling (BLOCK, SLIDE, PUSH)
  - ✅ Added focused debug logging to track collision detection and resolution
  - ✅ Enhanced enemy collision detection with detailed box logging
  - ✅ Implemented ultra simple collision solution with guaranteed escape from trapped situations
  - ✅ Added detailed debug logging for collision tracking and troubleshooting

## Development Setup
- Genesis/Megadrive development environment
- Resource compilation pipeline
- Build system for ROM generation
- Debugging tools for Genesis hardware

## Code Conventions
- Data types defined in Sega Genesis libraries are used: u8, u16, s8, s16, etc.
- All include statements are centralized in `globals.h`.
- Other header files (.h) do not require include statements.
- C files (.c) only include `globals.h`.

## Next Steps
1. Complete remaining Act 1 scenes
2. Polish existing gameplay mechanics
   - Optimize collision detection system (see `collision_system_optimization_plan.md`)
3. Implement additional spells and patterns
4. Enhance combat system
    - Refactor Combat System (Propuesta 1: Sistema de Estados y Mensajes)
        - Phase 1: Preparation ✅ (Document current system, identify dependencies, create basic tests, backup code)
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
        - Phase 2: Refactorización Base 🔄 (En progreso)
            - ✅ Creación inicial de la biblioteca de máquina de estados en `statemachine.h` y `statemachine.c`
            - ✅ Expandir la implementación actual de `statemachine.h` y `statemachine.c`:
                - ✅ Actualizar el enum `SM_State` para incluir: `SM_STATE_IDLE`, `SM_STATE_PLAYING_NOTE`, `SM_STATE_PATTERN_CHECK`, `SM_STATE_PATTERN_EFFECT`, `SM_STATE_PATTERN_EFFECT_FINISH`, `SM_STATE_ATTACK_FINISHED`
                - ✅ Expandir el enum `MessageType` para incluir: `MSG_PATTERN_COMPLETE`, `MSG_COMBAT_START`, `MSG_COMBAT_END`, `MSG_ENEMY_DEFEATED`, `MSG_PLAYER_HIT`, `MSG_ENEMY_HIT`, `MSG_NOTE_PLAYED`, `MSG_PATTERN_TIMEOUT`
                - ✅ Expandir la estructura `StateMachine` para incluir: `current_state`, `timer`, `notes[4]`, `note_count`, `current_note`, `note_time`, `pattern_time`, `active_pattern`, `is_reversed`, `effect_time`, `entity_id`
                - ✅ Actualizar las funciones `StateMachine_Init` y `StateMachine_Update`
                - ✅ Añadir función `StateMachine_SendMessage`
            - ✅ Crear documentación detallada de la máquina de estados (ver `memory_bank/state_machine_implementation_plan.md`)
            - 🔄 Pendiente: Mover variables globales relacionadas con el combate a la estructura `StateMachine`
            - 🔄 Pendiente: Incluir los archivos de cabecera necesarios
        - Phase 3: Implementación Core ⏳ (Pendiente)
            - Modificar `character_patterns.c`, `enemies_patterns.c`, y `combat.c` para usar la máquina de estados
            - Implementar función `combat_update` para manejar transiciones de estado basadas en mensajes
            - Implementar funciones para enviar mensajes
            - Modificar `enemies.c` para reaccionar a mensajes de combate
        - Phase 4: Finalization ⏳ (Pendiente)
            - Pruebas exhaustivas de todos los escenarios de combate
            - Asegurar compatibilidad con SGDK
            - Optimizar rendimiento
            - Documentación final y limpieza de código
5. Add more character interactions and story elements
