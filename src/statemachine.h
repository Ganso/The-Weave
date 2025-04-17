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