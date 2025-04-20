# Sistema de Pruebas de la Máquina de Estados

## Descripción General

El sistema de pruebas está diseñado para verificar el funcionamiento correcto de la máquina de estados antes de realizar modificaciones. Incluye pruebas unitarias para cada componente y funcionalidad.

## Archivos del Sistema

- `src/test_statemachine.h`: Declaraciones de funciones de prueba
- `src/test_statemachine.c`: Implementación de las pruebas

## Cómo Ejecutar las Pruebas

### 1. Compilación

Añadir los siguientes archivos al makefile:

```makefile
SRCS += $(SRCDIR)/test_statemachine.c
```

### 2. Ejecución

Para ejecutar las pruebas, llamar a la función principal de pruebas:

```c
void main() {
    // ... inicialización del sistema ...
    
    // Ejecutar pruebas
    run_state_machine_tests();
    
    // ... resto del código ...
}
```

## Interpretación de Resultados

Las pruebas mostrarán resultados en la consola de depuración usando KLog:

```
Starting State Machine Tests...
✓ PASS: State Machine Initialization
✓ PASS: State Transitions
✗ FAIL: Message Handling
  Error: Failed to handle MSG_NOTE_PLAYED
✓ PASS: Context Behavior
✓ PASS: Timing Behavior

Test Summary: 4/5 passed
```

- ✓ PASS: La prueba se completó exitosamente
- ✗ FAIL: La prueba falló, con un mensaje de error explicativo

## Añadir Nuevas Pruebas

### 1. Definir la Prueba

En `test_statemachine.h`:
```c
TestResult test_new_feature();
```

### 2. Implementar la Prueba

En `test_statemachine.c`:
```c
TestResult test_new_feature() {
    TestResult result = {
        .test_name = "Nueva Característica",
        .passed = true,
        .message = NULL
    };
    
    setup_test_environment();
    
    // Implementar prueba aquí
    
    cleanup_test_environment();
    return result;
}
```

### 3. Añadir al Runner Principal

En `run_state_machine_tests()`:
```c
TestResult results[] = {
    // ... pruebas existentes ...
    test_new_feature()
};
```

## Funciones Auxiliares

### Simulación de Tiempo
```c
// Simula el paso de frames
simulate_time_passage(u16 frames);
```

### Verificación de Estado
```c
// Verifica el estado actual
bool verify_state(StateMachine *sm, SM_State expected_state);
```

### Secuencias de Notas
```c
// Reproduce una secuencia de notas
void play_note_sequence(StateMachine *sm, u8 *notes, u8 count);
```

## Patrones Comunes de Depuración

### 1. Fallo en Transición de Estado
```c
// Verificar estado actual
if (!verify_state(&test_sm, EXPECTED_STATE)) {
    KLog_U1("Estado actual: ", test_sm.current_state);
    KLog_U1("Estado esperado: ", EXPECTED_STATE);
}
```

### 2. Problemas de Timing
```c
// Registrar tiempos
KLog_U1("Tiempo transcurrido: ", test_sm.timer);
KLog_U1("Tiempo máximo: ", MAX_TIME);
```

### 3. Verificación de Mensajes
```c
// Verificar procesamiento de mensaje
Message msg = create_test_message(TYPE, PARAM);
StateMachine_Update(&test_sm, &msg);
verify_message_effects(&test_sm);
```

## Mejores Prácticas

1. **Aislamiento**: Cada prueba debe ser independiente
2. **Limpieza**: Usar setup_test_environment() y cleanup_test_environment()
3. **Claridad**: Mensajes de error descriptivos
4. **Cobertura**: Probar casos normales y límite

## Resolución de Problemas

### Problema: Prueba Falla Intermitentemente
- Verificar dependencias de tiempo
- Comprobar limpieza del entorno
- Revisar variables globales

### Problema: Estado Inesperado
- Imprimir estado actual
- Verificar transiciones
- Comprobar mensajes procesados

## Próximos Pasos

1. Ejecutar pruebas antes de cada modificación
2. Añadir nuevas pruebas para nuevas características
3. Mantener documentación actualizada
4. Revisar y mejorar cobertura de pruebas

## Notas

- Las pruebas son fundamentales antes de modificar el código
- Cada nueva característica debe incluir pruebas
- Mantener el sistema de pruebas actualizado
- Usar los resultados para guiar el desarrollo