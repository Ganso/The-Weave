# Plan de Transición de la Máquina de Estados

## Contexto Actual

### Estado Actual
- Máquina de estados en entity.h
- Framework de pruebas implementado
- Problemas de includes pendientes

### Objetivo
Transición gradual y segura desde la máquina de estados actual en entity.h hacia la nueva implementación en statemachine.h, manteniendo la funcionalidad en todo momento.

## Fases de Transición

### Fase 1: Preparación y Testing

#### 1.1 Resolución de Includes
```c
// Orden correcto de includes
#include <genesis.h>
#include "globals.h"
#include "test.h"
```

#### 1.2 Tests Iniciales
```c
// Tests del sistema actual
TestResult test_current_entity_states();
TestResult test_current_combat_flow();
TestResult test_current_pattern_system();
```

### Fase 2: Desacoplamiento de Estados

#### 2.1 Separación de Estados de Combate
```c
// En entity.h - Mantener solo estados base
typedef enum {
    STATE_IDLE,
    STATE_WALKING,
    STATE_FOLLOWING
} GameState;

// En statemachine.h - Estados de combate y patrones
typedef enum {
    SM_STATE_IDLE,
    SM_STATE_PLAYING_NOTE,
    SM_STATE_PATTERN_CHECK,
    SM_STATE_PATTERN_EFFECT,
    SM_STATE_PATTERN_EFFECT_FINISH,
    SM_STATE_ATTACK_FINISHED
} SM_State;
```

#### 2.2 Tests de Verificación
```c
// Verificar separación limpia
TestResult test_base_game_states();
TestResult test_combat_states();
TestResult test_state_independence();
```

### Fase 3: Implementación Nueva Máquina

#### 3.1 Sistema de Mensajes
```c
typedef enum {
    MSG_PATTERN_COMPLETE,
    MSG_COMBAT_START,
    MSG_COMBAT_END,
    MSG_ENEMY_DEFEATED,
    MSG_PLAYER_HIT,
    MSG_ENEMY_HIT,
    MSG_NOTE_PLAYED,
    MSG_PATTERN_TIMEOUT
} MessageType;
```

#### 3.2 Tests de Mensajes
```c
TestResult test_message_handling();
TestResult test_message_flow();
TestResult test_state_transitions();
```

### Fase 4: Integración Gradual

#### 4.1 Combat System
```c
void start_combat(bool start) {
    if (start) {
        is_combat_active = true;
        StateMachine_SendMessage(&combat_sm, MSG_COMBAT_START, 0);
    } else {
        StateMachine_SendMessage(&combat_sm, MSG_COMBAT_END, 0);
        is_combat_active = false;
    }
}
```

#### 4.2 Tests de Integración
```c
TestResult test_combat_initialization();
TestResult test_pattern_execution();
TestResult test_combat_flow();
```

## Plan de Testing

### 1. Tests Base
- Estado inicial correcto
- Transiciones básicas
- Manejo de mensajes

### 2. Tests de Transición
- Compatibilidad con sistema actual
- No regresiones
- Estados intermedios válidos

### 3. Tests de Integración
- Flujo de combate completo
- Patrones y efectos
- UI y feedback

## Verificación por Fase

### Fase 1: Preparación
- [x] Framework de pruebas funcionando
- [ ] Includes resueltos
- [ ] Tests base ejecutándose

### Fase 2: Desacoplamiento
- [ ] Estados separados correctamente
- [ ] No hay dependencias circulares
- [ ] Tests de separación pasan

### Fase 3: Nueva Implementación
- [ ] Sistema de mensajes funcionando
- [ ] Máquina de estados operativa
- [ ] Tests de comportamiento pasan

### Fase 4: Integración
- [ ] Sistema de combate usando nueva máquina
- [ ] Patrones funcionando correctamente
- [ ] Tests de integración pasan

## Consideraciones Técnicas

### 1. Compatibilidad
- Mantener tipos SGDK
- Respetar limitaciones de memoria
- Considerar performance

### 2. Testing
- Tests independientes
- Cobertura completa
- Fácil mantenimiento

### 3. Documentación
- Actualizar a medida que se avanza
- Documentar decisiones clave
- Mantener diagramas actualizados

## Próximos Pasos

### Inmediato
1. Resolver problemas de includes
2. Implementar tests base
3. Comenzar desacoplamiento

### Corto Plazo
1. Completar separación de estados
2. Implementar sistema de mensajes
3. Verificar comportamiento base

### Medio Plazo
1. Integrar con sistema de combate
2. Expandir cobertura de tests
3. Optimizar performance

## Métricas de Éxito

### 1. Funcionalidad
- Todos los tests pasan
- No hay regresiones
- Comportamiento correcto

### 2. Calidad
- Cobertura de código >90%
- No memory leaks
- Performance mantenida

### 3. Mantenibilidad
- Código documentado
- Estructura clara
- Fácil de extender

## Referencias

1. Combat System Proposal
2. SGDK Documentation
3. Testing Best Practices

## Notas

- Mantener backup del código actual
- Documentar cada paso de la transición
- Verificar constantemente no hay regresiones
- Considerar rollback si necesario