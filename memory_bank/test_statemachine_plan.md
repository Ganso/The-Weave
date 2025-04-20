# Plan de Pruebas para la Máquina de Estados

## Estructura de la Biblioteca de Pruebas

### 1. Archivos Necesarios
```
src/
  test_statemachine.h     // Declaraciones de funciones de prueba
  test_statemachine.c     // Implementación de las pruebas
```

### 2. Funciones de Prueba

#### Pruebas de Inicialización
```c
// Verifica la correcta inicialización de StateMachine
void test_state_machine_init() {
    // Verificar valores iniciales
    // Comprobar array de notas
    // Validar estado inicial
}
```

#### Pruebas de Transiciones
```c
// Verifica todas las transiciones posibles entre estados
void test_state_transitions() {
    // IDLE -> PLAYING_NOTE
    // PLAYING_NOTE -> PATTERN_CHECK
    // PATTERN_CHECK -> PATTERN_EFFECT
    // etc.
}
```

#### Pruebas de Mensajes
```c
// Verifica el manejo de mensajes
void test_message_handling() {
    // MSG_PATTERN_COMPLETE
    // MSG_COMBAT_START/END
    // MSG_NOTE_PLAYED
    // etc.
}
```

#### Pruebas de Contexto
```c
// Verifica comportamientos en diferentes contextos
void test_context_behavior() {
    // Comportamiento en IDLE (normal vs combate)
    // Comportamiento en PLAYING_NOTE (normal vs combate)
    // etc.
}
```

#### Pruebas de Timers
```c
// Verifica la gestión de tiempos
void test_timing_behavior() {
    // MAX_NOTE_PLAYING_TIME
    // MAX_PATTERN_WAIT_TIME
    // MAX_EFFECT_TIME
    // etc.
}
```

### 3. Funciones Auxiliares

```c
// Simula el paso del tiempo
void simulate_time_passage(u16 frames);

// Verifica el estado actual
bool verify_state(StateMachine *sm, SM_State expected_state);

// Simula una secuencia de notas
void play_note_sequence(StateMachine *sm, u8 *notes, u8 count);

// Verifica efectos activos
bool verify_effects(StateMachine *sm, u16 expected_pattern);
```

## Casos de Prueba

### 1. Pruebas Básicas
- Inicialización correcta
- Estado inicial es IDLE
- Array de notas vacío
- Timers en 0

### 2. Pruebas de Transición
- IDLE -> PLAYING_NOTE al recibir nota
- PLAYING_NOTE -> PATTERN_CHECK al completar tiempo
- PATTERN_CHECK -> PATTERN_EFFECT con patrón válido
- PATTERN_CHECK -> IDLE sin patrón válido
- PATTERN_EFFECT -> PATTERN_EFFECT_FINISH al completar
- PATTERN_EFFECT_FINISH -> IDLE al finalizar

### 3. Pruebas de Mensajes
- MSG_PATTERN_COMPLETE activa efecto
- MSG_COMBAT_START/END cambia contexto
- MSG_NOTE_PLAYED inicia reproducción
- MSG_PATTERN_TIMEOUT limpia estado

### 4. Pruebas de Contexto
- Comportamiento IDLE en modo normal
- Comportamiento IDLE en combate
- Efectos de patrón en modo normal
- Efectos de patrón en combate

### 5. Pruebas de Tiempo
- Respetar MAX_NOTE_PLAYING_TIME
- Respetar MAX_PATTERN_WAIT_TIME
- Respetar MAX_EFFECT_TIME
- Respetar MAX_TIME_AFTER_ATTACK

## Implementación

### Paso 1: Estructura Base
1. Crear archivos test_statemachine.h/.c
2. Definir funciones de prueba básicas
3. Implementar funciones auxiliares

### Paso 2: Pruebas Unitarias
1. Implementar cada función de prueba
2. Añadir verificaciones detalladas
3. Documentar resultados esperados

### Paso 3: Integración
1. Conectar con sistema principal
2. Verificar no hay efectos secundarios
3. Documentar cualquier problema

## Uso de las Pruebas

### Ejecución
```c
// Ejemplo de uso
void run_state_machine_tests() {
    test_state_machine_init();
    test_state_transitions();
    test_message_handling();
    test_context_behavior();
    test_timing_behavior();
}
```

### Verificación
1. Todas las pruebas deben pasar
2. No debe haber efectos secundarios
3. El rendimiento debe ser aceptable

## Próximos Pasos

1. Implementar la biblioteca de pruebas en Code mode
2. Ejecutar pruebas del sistema actual
3. Usar resultados para guiar mejoras
4. Mantener pruebas actualizadas con cambios

## Notas Importantes

- Las pruebas deben ser deterministas
- Cada prueba debe ser independiente
- Documentar todos los casos de fallo
- Mantener registro de resultados