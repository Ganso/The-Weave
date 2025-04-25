# Plan de Transición de Variables Globales a StateMachine

## Objetivo
Migrar las variables globales relacionadas con el combate a la estructura StateMachine de manera gradual y segura, manteniendo la funcionalidad existente en cada paso.

## Fase 1: Migración de Variables de character_patterns.c

### Variables a Migrar
```c
// Desde character_patterns.c
bool player_patterns_enabled;
bool note_playing;
u16 note_playing_time;
u16 time_since_last_note;
bool player_pattern_effect_in_progress;
bool player_pattern_effect_reversed;
u16 player_pattern_effect_time;
u8 played_notes[4];
u8 num_played_notes;
Pattern obj_pattern[MAX_PATTERNS];
```

### Estructura StateMachine Expandida
```c
typedef struct {
    // Campos existentes
    SM_State current_state;
    u16 timer;
    u8 notes[4];
    u8 note_count;
    u8 current_note;
    u16 note_time;
    u16 pattern_time;
    u16 active_pattern;
    bool is_reversed;
    u16 effect_time;
    u16 entity_id;

    // Nuevos campos
    bool patterns_enabled;
    bool is_note_playing;
    u16 time_since_last_note;
    bool effect_in_progress;
    Pattern *patterns;        // Array de patrones
    u8 pattern_count;        // Número de patrones disponibles
} StateMachine;
```

### Pasos de Implementación

1. Actualizar statemachine.h:
   - Añadir los nuevos campos a la estructura
   - Definir constantes necesarias
   - Actualizar la documentación

2. Actualizar statemachine.c:
   - Modificar StateMachine_Init para inicializar los nuevos campos
   - Actualizar StateMachine_Update para usar los nuevos campos
   - Añadir funciones auxiliares si son necesarias

3. Modificar character_patterns.c:
   - Crear una instancia de StateMachine para el jugador
   - Reemplazar gradualmente el uso de variables globales por accesos a la estructura
   - Mantener las variables globales temporalmente como referencias a los campos de StateMachine

### Ejemplo de Código de Transición

```c
// En character_patterns.c
StateMachine player_state_machine;

// Función de inicialización
void init_character_patterns() {
    StateMachine_Init(&player_state_machine, PLAYER_ENTITY_ID);
    player_state_machine.patterns_enabled = true;
    // ... resto de la inicialización
}

// Función de actualización
void update_character_patterns() {
    if (player_state_machine.is_note_playing) {
        player_state_machine.note_time++;
        // ... resto de la lógica
    }
}
```

## Beneficios de Esta Fase

1. Encapsulación más clara del estado del jugador
2. Eliminación gradual de variables globales
3. Mejor mantenibilidad del código
4. Base sólida para las siguientes fases de migración

## Siguientes Fases

1. Fase 2: Migración de variables de enemies_patterns.c
2. Fase 3: Migración de variables de combat.c
3. Fase 4: Migración de variables de enemies.c

## Consideraciones

1. Mantener compatibilidad hacia atrás durante la transición
2. Realizar la migración de forma incremental y testeable
3. Documentar cada cambio en el código
4. Actualizar las pruebas existentes según sea necesario

## Plan de Pruebas

1. Verificar que cada patrón funciona correctamente después de la migración
2. Comprobar que las transiciones de estado son correctas
3. Asegurar que el timing de las notas y efectos se mantiene
4. Validar que no hay efectos secundarios en otras partes del sistema

## Riesgos y Mitigación

1. **Riesgo**: Pérdida de sincronización en el timing de las notas
   - **Mitigación**: Implementar verificaciones de timing y logging temporal

2. **Riesgo**: Conflictos con el código existente
   - **Mitigación**: Mantener temporalmente las variables globales como referencias

3. **Riesgo**: Problemas de rendimiento
   - **Mitigación**: Monitorizar el rendimiento durante la migración