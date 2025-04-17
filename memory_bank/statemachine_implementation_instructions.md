# Instrucciones para Implementar la Máquina de Estados

Este documento proporciona instrucciones detalladas para implementar los cambios necesarios en `statemachine.h` y `statemachine.c` como primer paso para la refactorización del sistema de combate.

## Cambios en statemachine.h

Reemplaza el contenido actual de `statemachine.h` con el siguiente código:

```c
#ifndef STATEMACHINE_H
#define STATEMACHINE_H

// Definición de constantes para tiempos máximos
#define MAX_NOTE_PLAYING_TIME  500  // Tiempo máximo de reproducción de nota en milisegundos
#define MAX_PATTERN_WAIT_TIME 2000  // Tiempo máximo de espera para la siguiente nota en milisegundos
#define MAX_EFFECT_TIME       1600  // Tiempo máximo para efectos de patrón en milisegundos
#define MAX_TIME_AFTER_ATTACK 1000  // Tiempo máximo después de un ataque en milisegundos

// Estados de la máquina de estados
typedef enum {
    SM_STATE_IDLE,               // Sistema inactivo
    SM_STATE_PLAYING_NOTE,       // Reproduciendo una nota
    SM_STATE_PATTERN_CHECK,      // Verificando si el patrón es válido
    SM_STATE_PATTERN_EFFECT,     // Ejecutando el efecto del patrón
    SM_STATE_PATTERN_EFFECT_FINISH, // Finalizando el efecto del patrón
    SM_STATE_ATTACK_FINISHED     // Ataque finalizado, en enfriamiento
} SM_State;

// Tipos de mensajes para comunicación entre componentes
typedef enum {
    MSG_PATTERN_COMPLETE,        // Patrón completado
    MSG_COMBAT_START,            // Inicio de combate
    MSG_COMBAT_END,              // Fin de combate
    MSG_ENEMY_DEFEATED,          // Enemigo derrotado
    MSG_PLAYER_HIT,              // Jugador golpeado
    MSG_ENEMY_HIT,               // Enemigo golpeado
    MSG_NOTE_PLAYED,             // Nota reproducida
    MSG_PATTERN_TIMEOUT          // Tiempo de espera del patrón agotado
} MessageType;

// Estructura de mensaje para comunicación
typedef struct {
    MessageType type;            // Tipo de mensaje
    u16 param;                   // Parámetro adicional (depende del tipo de mensaje)
} Message;

// Estructura principal de la máquina de estados
typedef struct {
    SM_State current_state;      // Estado actual
    u16 timer;                   // Temporizador general
    u8 notes[4];                 // Notas del patrón actual
    u8 note_count;               // Número de notas reproducidas
    u8 current_note;             // Nota actual que se está reproduciendo
    u16 note_time;               // Tiempo que lleva reproduciendo la nota
    u16 pattern_time;            // Tiempo desde la última nota
    u16 active_pattern;          // Patrón activo (si hay alguno)
    bool is_reversed;            // Si el patrón es invertido
    u16 effect_time;             // Tiempo que lleva el efecto activo
    u16 entity_id;               // ID de la entidad (jugador o enemigo)
} StateMachine;

// Funciones de la máquina de estados
void StateMachine_Init(StateMachine *sm, u16 entity_id);
void StateMachine_Update(StateMachine *sm, Message *msg);
void StateMachine_SendMessage(StateMachine *sm, MessageType type, u16 param);

#endif
```

## Cambios en statemachine.c

Reemplaza el contenido actual de `statemachine.c` con el siguiente código:

```c
#include "globals.h"

/**
 * Inicializa una máquina de estados.
 * 
 * @param sm Puntero a la estructura StateMachine a inicializar
 * @param entity_id ID de la entidad asociada a esta máquina de estados
 */
void StateMachine_Init(StateMachine *sm, u16 entity_id) {
    sm->current_state = SM_STATE_IDLE;
    sm->timer = 0;
    sm->note_count = 0;
    sm->current_note = 0;
    sm->note_time = 0;
    sm->pattern_time = 0;
    sm->active_pattern = 0;
    sm->is_reversed = false;
    sm->effect_time = 0;
    sm->entity_id = entity_id;
    
    // Inicializar array de notas
    for (u8 i = 0; i < 4; i++) {
        sm->notes[i] = 0;
    }
}

/**
 * Actualiza el estado de una máquina de estados basado en un mensaje.
 * 
 * @param sm Puntero a la estructura StateMachine a actualizar
 * @param msg Puntero al mensaje que se procesará (puede ser NULL)
 */
void StateMachine_Update(StateMachine *sm, Message *msg) {
    // Lógica de actualización para cada estado
    switch (sm->current_state) {
        case SM_STATE_IDLE:
            // Manejar mensajes en estado IDLE
            if (msg != NULL) {
                switch (msg->type) {
                    case MSG_COMBAT_START:
                        sm->current_state = SM_STATE_PLAYING_NOTE;
                        sm->timer = 0;
                        break;
                    case MSG_NOTE_PLAYED:
                        sm->current_state = SM_STATE_PLAYING_NOTE;
                        sm->current_note = msg->param;
                        sm->notes[sm->note_count++] = msg->param;
                        sm->note_time = 0;
                        break;
                    default:
                        break;
                }
            }
            break;
            
        case SM_STATE_PLAYING_NOTE:
            // Lógica para reproducir notas
            sm->note_time++;
            if (sm->note_time > MAX_NOTE_PLAYING_TIME) {
                sm->current_state = SM_STATE_PATTERN_CHECK;
                sm->note_time = 0;
            }
            break;
            
        case SM_STATE_PATTERN_CHECK:
            // Aquí iría la lógica para verificar patrones
            // Por ahora, simplemente transicionamos a IDLE o EFFECT
            if (msg != NULL && msg->type == MSG_PATTERN_COMPLETE) {
                sm->current_state = SM_STATE_PATTERN_EFFECT;
                sm->active_pattern = msg->param;
                sm->effect_time = 0;
            } else {
                sm->current_state = SM_STATE_IDLE;
            }
            break;
            
        case SM_STATE_PATTERN_EFFECT:
            // Lógica para efectos de patrones
            sm->effect_time++;
            if (sm->effect_time > MAX_EFFECT_TIME) {
                sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
                sm->effect_time = 0;
            }
            break;
            
        case SM_STATE_PATTERN_EFFECT_FINISH:
            // Lógica para finalizar efectos
            sm->timer++;
            if (sm->timer > 20) {
                sm->current_state = SM_STATE_IDLE;
                sm->timer = 0;
                sm->active_pattern = 0;
            }
            break;
            
        case SM_STATE_ATTACK_FINISHED:
            // Lógica para el estado post-ataque
            sm->timer++;
            if (sm->timer > MAX_TIME_AFTER_ATTACK) {
                sm->current_state = SM_STATE_IDLE;
                sm->timer = 0;
            }
            break;
            
        default:
            // Manejar estado desconocido
            sm->current_state = SM_STATE_IDLE;
            sm->timer = 0;
            break;
    }
}

/**
 * Envía un mensaje a una máquina de estados para su procesamiento.
 * 
 * @param sm Puntero a la estructura StateMachine que recibirá el mensaje
 * @param type Tipo de mensaje a enviar
 * @param param Parámetro adicional del mensaje
 */
void StateMachine_SendMessage(StateMachine *sm, MessageType type, u16 param) {
    Message msg;
    msg.type = type;
    msg.param = param;
    StateMachine_Update(sm, &msg);
}
```

## Notas Importantes

1. **Compatibilidad**: Esta implementación es compatible con el código existente, ya que mantiene las funciones originales mientras añade nuevas funcionalidades.

2. **Constantes**: Se han definido constantes para los tiempos máximos basados en los valores existentes en el código actual.

3. **Nuevos Estados**: Se han añadido los estados `SM_STATE_PLAYING_NOTE`, `SM_STATE_PATTERN_CHECK` y `SM_STATE_ATTACK_FINISHED` para manejar todas las fases del sistema de combate.

4. **Nuevos Mensajes**: Se han añadido los mensajes `MSG_COMBAT_END`, `MSG_PLAYER_HIT`, `MSG_ENEMY_HIT`, `MSG_NOTE_PLAYED` y `MSG_PATTERN_TIMEOUT` para una comunicación más completa entre componentes.

5. **Nueva Función**: Se ha añadido la función `StateMachine_SendMessage` para facilitar el envío de mensajes a la máquina de estados.

## Próximos Pasos

Una vez implementados estos cambios, los siguientes pasos serían:

1. Integrar la máquina de estados con el sistema de patrones del personaje
2. Integrar la máquina de estados con el sistema de patrones de enemigos
3. Crear una función combat_update en combat.c que utilice la máquina de estados
4. Realizar pruebas exhaustivas para asegurar que todo funciona correctamente