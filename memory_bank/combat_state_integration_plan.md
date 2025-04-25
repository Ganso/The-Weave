# Plan de Integración del Sistema de Estados de Combate

## Fase 1: Preparación de la Estructura

### 1.1 Mapeo de Estados
```c
// Mapeo entre estados actuales y nuevos
typedef enum {
    SM_STATE_IDLE,               // Mapea a STATE_IDLE
    SM_STATE_PLAYING_NOTE,       // Mapea a STATE_PLAYING_NOTE
    SM_STATE_PATTERN_CHECK,      // Mapea a STATE_PATTERN_CHECK
    SM_STATE_PATTERN_EFFECT,     // Mapea a STATE_PATTERN_EFFECT
    SM_STATE_PATTERN_EFFECT_FINISH, // Mapea a STATE_PATTERN_EFFECT_FINISH
    SM_STATE_ATTACK_FINISHED     // Nuevo estado para manejo post-ataque
} SM_State;
```

### 1.2 Estructura StateMachine Expandida
```c
typedef struct {
    // Estados y temporizadores base
    SM_State current_state;
    u16 timer;
    
    // Sistema de notas
    u8 notes[4];
    u8 note_count;
    u8 current_note;
    u16 note_time;
    u16 pattern_time;
    
    // Sistema de patrones
    u16 active_pattern;
    bool is_reversed;
    u16 effect_time;
    u16 entity_id;
    
    // Nuevos campos para patrones específicos
    struct {
        bool enabled;
        bool is_note_playing;
        u16 time_since_last_note;
        bool effect_in_progress;
        u16 effect_type;         // PTRN_NONE, PTRN_ELECTRIC, etc.
        bool effect_reversed;
        u16 effect_duration;
        Pattern *available_patterns;
        u8 pattern_count;
    } pattern_system;
    
    // Callbacks para efectos específicos
    void (*launch_effect)(struct StateMachine*);
    void (*do_effect)(struct StateMachine*);
    void (*finish_effect)(struct StateMachine*);
} StateMachine;
```

## Fase 2: Implementación Gradual

### 2.1 Crear Funciones de Transición
```c
// Función para convertir estado actual a SM_State
SM_State convert_to_sm_state(u16 current_state) {
    switch(current_state) {
        case STATE_IDLE: return SM_STATE_IDLE;
        case STATE_PLAYING_NOTE: return SM_STATE_PLAYING_NOTE;
        // etc.
    }
}

// Función para actualizar estado del personaje desde SM_State
void update_character_from_sm_state(Character* chr, SM_State state) {
    switch(state) {
        case SM_STATE_IDLE: chr->state = STATE_IDLE; break;
        case SM_STATE_PLAYING_NOTE: chr->state = STATE_PLAYING_NOTE; break;
        // etc.
    }
}
```

### 2.2 Implementar Funciones de Callback
```c
// Ejemplo para el patrón eléctrico
void launch_electric_effect(StateMachine* sm) {
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_ELECTRIC, true, true);
    play_pattern_sound(PTRN_ELECTRIC);
    sm->pattern_system.effect_type = PTRN_ELECTRIC;
}

void do_electric_effect(StateMachine* sm) {
    // Implementar efecto visual de trueno
    // Aplicar efectos de combate
}

void finish_electric_effect(StateMachine* sm) {
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_duration = 0;
}
```

## Fase 3: Plan de Migración

1. **Paso 1: Crear Instancia de StateMachine**
   ```c
   StateMachine player_sm;
   StateMachine_Init(&player_sm, active_character);
   ```

2. **Paso 2: Migrar Variables Globales**
   ```c
   // Antes
   bool player_patterns_enabled;
   // Después
   player_sm.pattern_system.enabled = true;
   ```

3. **Paso 3: Actualizar check_active_character_state**
   ```c
   void check_active_character_state(void) {
       // Obtener estado de la máquina de estados
       SM_State new_state = StateMachine_Update(&player_sm, NULL);
       
       // Actualizar estado del personaje
       update_character_from_sm_state(&obj_character[active_character], new_state);
       
       // Procesar efectos si es necesario
       if (player_sm.pattern_system.effect_in_progress && player_sm.do_effect) {
           player_sm.do_effect(&player_sm);
       }
   }
   ```

## Fase 4: Plan de Pruebas

1. **Pruebas de Transición de Estados**
   - Verificar que cada estado se mapea correctamente
   - Comprobar que las animaciones se mantienen sincronizadas
   - Validar el timing de las notas

2. **Pruebas de Patrones**
   - Probar cada patrón individualmente
   - Verificar efectos visuales y de combate
   - Comprobar interacciones entre patrones

3. **Pruebas de Integración**
   - Validar la integración con el sistema de combate
   - Comprobar la interacción con enemigos
   - Verificar el manejo de eventos del juego

## Consideraciones de Implementación

1. **Mantener Compatibilidad**
   - Usar defines para facilitar la transición
   - Mantener funciones wrapper temporales
   - Documentar cambios en la API

2. **Gestión de Memoria**
   - Asegurar que los patrones se inicializan correctamente
   - Liberar recursos apropiadamente
   - Manejar límites de memoria de la Genesis

3. **Rendimiento**
   - Monitorizar el uso de CPU
   - Optimizar accesos a memoria
   - Mantener el rendimiento del juego

## Próximos Pasos

1. Implementar la estructura StateMachine expandida
2. Crear las funciones de transición
3. Migrar un patrón como prueba de concepto
4. Evaluar resultados y ajustar el plan según sea necesario