# Tests de la Máquina de Estados

## Test 1: Estado y Transiciones Básicas (Existente)
```c
TestResult test_basic_state_machine() {
    // Ya implementado
    // Verifica:
    // - Estado inicial IDLE
    // - Transición a PLAYING_NOTE
    // - Datos de estado actualizados
}
```

## Test 2: Manejo de Tiempos
```c
TestResult test_timing_behavior() {
    // Verificar:
    // - MAX_NOTE_PLAYING_TIME (500ms)
    // - MAX_PATTERN_WAIT_TIME (2000ms)
    // - Transición automática después del tiempo
}
```

## Test 3: Secuencia de Estados
```c
TestResult test_state_sequence() {
    // Verificar secuencia completa:
    // IDLE -> PLAYING_NOTE -> PATTERN_CHECK -> PATTERN_EFFECT -> PATTERN_EFFECT_FINISH -> IDLE
    // Usando los mensajes apropiados
}
```

## Test 4: Manejo de Mensajes
```c
TestResult test_message_handling() {
    // Verificar mensajes:
    // - MSG_NOTE_PLAYED (con param = nota)
    // - MSG_PATTERN_COMPLETE (con param = patrón)
    // - MSG_PATTERN_TIMEOUT
}
```

## Implementación

### Estilo
- Usar KLog para output simple
- TEST_ASSERT_MESSAGE para verificaciones
- Setup/cleanup del entorno por test
- Mantener tests independientes

### Estructura Común
```c
TestResult test_example() {
    TestResult result = {
        .test_name = "Test Name",
        .passed = TRUE,
        .message = NULL
    };
    
    setup_test_environment();
    
    // Test steps
    KLog("Testing step...\n");
    TEST_ASSERT_MESSAGE(condition, "Error message");
    
    cleanup_test_environment();
    return result;
}
```

## Ejecución
```c
void run_state_machine_tests() {
    KLog("\n=== Running State Machine Tests ===\n");
    
    TestResult results[] = {
        test_basic_state_machine(),
        test_timing_behavior(),
        test_state_sequence(),
        test_message_handling()
    };
    
    // Mostrar resultados
    for (int i = 0; i < 4; i++) {
        print_test_result(results[i]);
    }
    
    KLog("\n=== Tests Complete ===\n");
}
```

## Aspectos a Verificar

### 1. Estructura StateMachine
- current_state inicializado correctamente
- timer a 0
- notes[] limpio
- note_count y current_note a 0
- tiempos a 0
- active_pattern a 0
- is_reversed a false

### 2. Transiciones de Estado
- IDLE -> PLAYING_NOTE (por MSG_NOTE_PLAYED)
- PLAYING_NOTE -> PATTERN_CHECK (por timeout)
- PATTERN_CHECK -> PATTERN_EFFECT (por MSG_PATTERN_COMPLETE)
- PATTERN_EFFECT -> PATTERN_EFFECT_FINISH (por MAX_EFFECT_TIME)
- PATTERN_EFFECT_FINISH -> IDLE (por timer)

### 3. Manejo de Tiempo
- note_time incrementa correctamente
- effect_time incrementa correctamente
- timer general funciona
- timeouts provocan transiciones correctas

### 4. Datos de Estado
- Notas se guardan correctamente
- Contador de notas incrementa bien
- Patrón activo se establece
- Entity ID se mantiene

## Notas
- Tests simples y directos
- Verificar solo funcionalidad esencial
- No probar casos extremos por ahora
- Mantener el estilo de código existente