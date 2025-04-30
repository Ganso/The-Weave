#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "globals.h"

// Forward declarations
struct StateMachine;

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

// Tipos de callbacks para efectos de patrones
typedef void (*EffectCallback)(struct StateMachine*);

// Sistema de patrones
typedef struct {
    bool enabled;               // Si el sistema de patrones está activo
    bool is_note_playing;       // Si hay una nota reproduciéndose
    u16 time_since_last_note;   // Tiempo desde la última nota
    bool effect_in_progress;    // Si hay un efecto en progreso
    u16 effect_type;           // Tipo de efecto activo (PTRN_NONE, PTRN_ELECTRIC, etc.)
    bool effect_reversed;       // Si el efecto está invertido
    u16 effect_duration;        // Duración del efecto actual
    Pattern* available_patterns;  // Array de patrones disponibles
    u8 pattern_count;          // Número de patrones disponibles
} PatternSystem;

// Estructura principal de la máquina de estados
typedef struct StateMachine {
    // Estados y temporizadores base
    SM_State current_state;      // Estado actual
    u16 timer;                   // Temporizador general
    
    // Sistema de notas
    u8 notes[4];                 // Notas del patrón actual
    u8 note_count;               // Número de notas reproducidas
    u8 current_note;             // Nota actual que se está reproduciendo
    u16 note_time;               // Tiempo que lleva reproduciendo la nota
    u16 pattern_time;            // Tiempo desde la última nota
    
    // Sistema de patrones base
    u16 active_pattern;          // Patrón activo (si hay alguno)
    bool is_reversed;            // Si el patrón es invertido
    u16 effect_time;             // Tiempo que lleva el efecto activo
    u16 entity_id;               // ID de la entidad (jugador o enemigo)
    
    // Sistema de patrones expandido
    PatternSystem pattern_system;
    
    // Callbacks para efectos específicos
    EffectCallback launch_effect;  // Función para iniciar un efecto
    EffectCallback do_effect;      // Función para procesar un efecto
    EffectCallback finish_effect;  // Función para finalizar un efecto
} StateMachine;

// Funciones de conversión de estados
SM_State convert_to_sm_state(u16 current_state);
void update_character_from_sm_state(Entity* entity, SM_State state);

// Funciones de la máquina de estados
void StateMachine_Init(StateMachine *sm, u16 entity_id);
void StateMachine_Update(StateMachine *sm, Message *msg);
void StateMachine_SendMessage(StateMachine *sm, MessageType type, u16 param);

// Funciones de efectos para el patrón eléctrico
void electric_pattern_launch(StateMachine* sm);
void electric_pattern_do(StateMachine* sm);
void electric_pattern_finish(StateMachine* sm);

// Funciones de efectos para el patrón de esconderse (HIDE)
void hide_pattern_launch(StateMachine* sm);
void hide_pattern_do(StateMachine* sm);
void hide_pattern_finish(StateMachine* sm);

// Funciones de efectos para el patrón de dormir (SLEEP)
void sleep_pattern_launch(StateMachine* sm);
void sleep_pattern_do(StateMachine* sm);
void sleep_pattern_finish(StateMachine* sm);

// Funciones de efectos para el patrón de abrir (OPEN)
void open_pattern_launch(StateMachine* sm);
void open_pattern_do(StateMachine* sm);
void open_pattern_finish(StateMachine* sm);

#endif