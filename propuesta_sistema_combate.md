# Propuesta de Refactorización: Sistema de Combate y Patrones

## Índice
1. [Situación Actual](#situación-actual)
2. [Objetivos](#objetivos)
3. [Propuestas de Arquitectura](#propuestas-de-arquitectura)
4. [Comparativa de Soluciones](#comparativa-de-soluciones)
5. [Plan de Implementación](#plan-de-implementación)
6. [Conclusiones](#conclusiones)

## Situación Actual

El sistema actual presenta los siguientes desafíos:

- Uso extensivo de variables globales
- Lógica de estados dispersa en múltiples archivos
- Alto acoplamiento entre sistemas
- Dificultad de mantenimiento
- Complejidad en el seguimiento del flujo de ejecución

Archivos principales afectados:
- character_patterns.c
- enemies_patterns.c
- combat.c
- enemies.c

## Objetivos

1. **Principales**
   - Reducir la complejidad del código
   - Mejorar la mantenibilidad
   - Mantener el rendimiento
   - Facilitar la depuración

2. **Técnicos**
   - Compatible con C estándar
   - Compatible con SGDK
   - Eficiente en uso de memoria
   - Optimizado para Megadrive

## Propuestas de Arquitectura

### Propuesta 1: Sistema de Estados y Mensajes

```c
// Sistema de Estados
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

// Sistema de Mensajes
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

#### Características
- Estados claramente definidos
- Sistema de mensajes ligero
- Agrupación de variables relacionadas
- Manejo centralizado de estados

### Propuesta 2: Subsistemas y Comandos

```c
// Subsistemas
typedef struct {
    void (*init)(void);
    void (*update)(void);
    void (*cleanup)(void);
    bool (*can_execute_pattern)(u8 pattern_id);
    void (*start_pattern)(u8 pattern_id);
    void (*cancel_pattern)(void);
} PatternSystem;

// Sistema de Comandos
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

#### Características
- Módulos independientes
- Sistema de comandos para acciones
- Gestor de recursos centralizado
- Datos organizados por subsistema

## Comparativa de Soluciones

| Característica | Sistema Actual | Propuesta 1 | Propuesta 2 |
|---------------|----------------|-------------|-------------|
| Complejidad | Alta | Media | Media |
| Mantenibilidad | Baja | Alta | Alta |
| Uso de Memoria | Medio | Bajo | Medio |
| Rendimiento | Alto | Alto | Alto |
| Facilidad de Debug | Baja | Media | Alta |
| Escalabilidad | Baja | Media | Alta |
| Acoplamiento | Alto | Bajo | Bajo |

### Ventajas y Desventajas

#### Propuesta 1: Estados y Mensajes
Pros:
- Más simple de implementar
- Menor uso de memoria
- Transición más suave desde sistema actual

Contras:
- Menos flexible que Propuesta 2
- Limitado a patrones predefinidos
- Menor capacidad de extensión

#### Propuesta 2: Subsistemas y Comandos
Pros:
- Más flexible y extensible
- Mejor organización del código
- Más fácil de depurar

Contras:
- Implementación más compleja
- Mayor uso de memoria
- Curva de aprendizaje más pronunciada

## Plan de Implementación

### Fase 1: Preparación
1. Documentar sistema actual
2. Identificar dependencias
3. Crear tests básicos
4. Backup del código

### Fase 2: Refactorización Base
1. Agrupar variables en estructuras
2. Centralizar definiciones
3. Crear archivos de cabecera
4. Documentar interfaces

### Fase 3: Implementación Core
1. Implementar sistema elegido
2. Migrar funcionalidad gradualmente
3. Verificar funcionamiento
4. Optimizar rendimiento

### Fase 4: Finalización
1. Pruebas completas
2. Documentación final
3. Limpieza de código
4. Verificación de rendimiento

### Ejemplo de Timeline

| Semana | Tarea | Descripción |
|--------|-------|-------------|
| 1 | Preparación | Documentación y análisis |
| 2 | Estructuras Base | Definición de tipos y estructuras |
| 3-4 | Core Sistema | Implementación principal |
| 5 | Migración | Transición de código existente |
| 6 | Testing | Pruebas y optimización |

## Conclusiones

### Recomendación
Se recomienda implementar la **Propuesta 1** inicialmente, con posibilidad de evolucionar a la Propuesta 2 en el futuro:

1. Menor impacto inicial
2. Más rápida de implementar
3. Mantiene rendimiento
4. Base sólida para futuras mejoras

### Beneficios Esperados
- Código más mantenible
- Mejor organización
- Más fácil de extender
- Mejor rendimiento a largo plazo

### Siguientes Pasos
1. Revisar y aprobar propuesta
2. Definir timeline detallado
3. Comenzar con fase de preparación
4. Implementar cambios gradualmente

### Consideraciones Finales
- Mantener compatibilidad con SGDK
- Priorizar rendimiento
- Documentar cambios
- Realizar pruebas continuas
