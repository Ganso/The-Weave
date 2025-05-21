#include "globals.h"
#include "patterns_registry.h"
#include "counter_spell.h"

// Mapeo entre estados del personaje y estados de la máquina
SM_State convert_to_sm_state(u16 current_state) {
    switch(current_state) {
        case STATE_IDLE:
            return SM_STATE_IDLE;
        case STATE_PLAYING_NOTE:
            return SM_STATE_PLAYING_NOTE;
        case STATE_PATTERN_CHECK:
            return SM_STATE_PATTERN_CHECK;
        case STATE_PATTERN_EFFECT:
            return SM_STATE_PATTERN_EFFECT;
        case STATE_PATTERN_EFFECT_FINISH:
            return SM_STATE_PATTERN_EFFECT_FINISH;
        default:
            return SM_STATE_IDLE;
    }
}

void update_character_from_sm_state(Entity* entity, SM_State state) {
    // Debug log for enemy entities
    bool is_enemy = false;
    u16 entity_id = 0;
    
    // Check if this is an enemy entity
    for (u16 i = 0; i < MAX_ENEMIES; i++) {
        if (entity == &obj_enemy[i].obj_character) {
            is_enemy = true;
            entity_id = i;
            break;
        }
    }
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("update_character_from_sm_state: enemy=%d, current_state=%d, new_state=%d, walking=%d",
                entity_id, entity->state, state, (entity->state == STATE_WALKING));
    }
    
    // Solo actualizar si no estamos en movimiento o si es un estado de efecto
    // Also skip updating if the entity is an enemy that's currently attacking and in a special state
    bool skip_update = false;
    
    // Only skip if the entity state and state machine state are different
    if (is_enemy && entity_id == enemy_attacking &&
        (entity->state == STATE_PLAYING_NOTE || entity->state == STATE_PATTERN_EFFECT) &&
        ((entity->state == STATE_PLAYING_NOTE && state != SM_STATE_PLAYING_NOTE) ||
         (entity->state == STATE_PATTERN_EFFECT && state != SM_STATE_PATTERN_EFFECT))) {
        kprintf("  SKIPPING state update for attacking enemy in state %d (sm_state: %d)", entity->state, state);
        skip_update = true;
    }
    
    if (!skip_update && (entity->state != STATE_WALKING ||
        state == SM_STATE_PATTERN_EFFECT ||
        state == SM_STATE_PATTERN_EFFECT_FINISH)) {
        
        // Actualizar el estado de la entidad
        switch(state) {
            case SM_STATE_IDLE:
                if (entity->state != STATE_WALKING) {
                    entity->state = STATE_IDLE;
                    if (is_enemy && entity_id == enemy_attacking) {
                        kprintf("  Setting enemy state to IDLE");
                    }
                }
                break;
            case SM_STATE_PLAYING_NOTE:
                entity->state = STATE_PLAYING_NOTE;
                if (is_enemy && entity_id == enemy_attacking) {
                    kprintf("  Setting enemy state to PLAYING_NOTE");
                }
                break;
            case SM_STATE_PATTERN_CHECK:
                entity->state = STATE_PATTERN_CHECK;
                if (is_enemy && entity_id == enemy_attacking) {
                    kprintf("  Setting enemy state to PATTERN_CHECK");
                }
                break;
            case SM_STATE_PATTERN_EFFECT:
                entity->state = STATE_PATTERN_EFFECT;
                if (is_enemy && entity_id == enemy_attacking) {
                    kprintf("  Setting enemy state to PATTERN_EFFECT");
                }
                break;
            case SM_STATE_PATTERN_EFFECT_FINISH:
                entity->state = STATE_PATTERN_EFFECT_FINISH;
                if (is_enemy && entity_id == enemy_attacking) {
                    kprintf("  Setting enemy state to PATTERN_EFFECT_FINISH");
                }
                break;
            default:
                if (is_enemy && entity_id == enemy_attacking) {
                    kprintf("  Unknown state: %d", state);
                }
                break;
        }
        
        // Actualizar la animación si es un personaje
        if (entity == &obj_character[active_character]) {
            update_character_animation();
        }
    }
}

/**
 * Inicializa una máquina de estados.
 * 
 * @param sm Puntero a la estructura StateMachine a inicializar
 * @param entity_id ID de la entidad asociada a esta máquina de estados
 */
void StateMachine_Init(StateMachine *sm, u16 entity_id) {
    kprintf("Initializing state machine - MAX_NOTE_PLAYING_TIME ticks: %d", calc_ticks(MAX_NOTE_PLAYING_TIME));
    // Inicializar campos base
    sm->current_state = SM_STATE_IDLE;
    sm->timer = 0;
    sm->note_count = 0;
    sm->current_note = 0;
    sm->note_time = 0;
    sm->pattern_time = 0;
    sm->active_pattern = 0;
    sm->is_reversed = false;
    sm->is_counter_spell = false;
    sm->effect_time = 0;
    sm->entity_id = entity_id;
    sm->owner_type = OWNER_PLAYER; // Default to player
    
    // Inicializar array de notas
    for (u8 i = 0; i < 4; i++) {
        sm->notes[i] = 0;
    }
    
    // Inicializar sistema de patrones
    sm->pattern_system.enabled = true;
    sm->pattern_system.is_note_playing = false;
    sm->pattern_system.time_since_last_note = 0;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_reversed = false;
    sm->pattern_system.effect_duration = 0;
    sm->pattern_system.available_patterns = NULL;
    sm->pattern_system.pattern_count = 0;
    
    // Inicializar callbacks
    sm->launch_effect = NULL;
    sm->do_effect = NULL;
    sm->finish_effect = NULL;
    sm->validate_pattern = NULL;
    sm->pattern_complete = NULL;
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
                        if (msg->param > 0) {
                            kprintf("Note played: %d", msg->param);
                        }
                        sm->current_state = SM_STATE_PLAYING_NOTE;
                        sm->current_note = msg->param;
                        sm->note_time = 0;
                        // Asegurarse de que el sistema de patrones sepa que estamos reproduciendo una nota
                        sm->pattern_system.is_note_playing = true;
                        break;
                    default:
                        break;
                }
            }
            break;
            
        case SM_STATE_PLAYING_NOTE:
            // Lógica para reproducir notas
            sm->note_time++;
            if (sm->note_time == calc_ticks(MAX_NOTE_PLAYING_TIME)) {
                sm->current_state = SM_STATE_PATTERN_CHECK;
                sm->note_time = 0;
                // Marcar que ya no estamos reproduciendo una nota
                sm->pattern_system.is_note_playing = false;
            }
            break;
            
        case SM_STATE_PATTERN_CHECK:
            // Verificar patrones y configurar callbacks según el tipo
            if (msg != NULL && msg->type == MSG_PATTERN_COMPLETE) {
                kprintf("Pattern complete: %d", msg->param);
                sm->current_state = SM_STATE_PATTERN_EFFECT;
                sm->active_pattern = msg->param;
                sm->effect_time = 0;
                
                // Obtener el patrón del registro
                Pattern* pattern = get_pattern(msg->param, sm->owner_type);
                if (pattern) {
                    // Configurar callbacks desde el patrón
                    sm->launch_effect = pattern->launch;
                    sm->do_effect = pattern->do_effect;
                    sm->finish_effect = pattern->finish;
                    
                    // Iniciar el efecto si hay un callback registrado
                    if (sm->launch_effect != NULL) {
                        sm->launch_effect(sm);
                    }
                } else {
                    // Configurar callbacks según el tipo de patrón (para compatibilidad)
                    switch (msg->param) {
                        case PTRN_ELECTRIC:
                            sm->launch_effect = electric_pattern_launch;
                            sm->do_effect = electric_pattern_do;
                            sm->finish_effect = electric_pattern_finish;
                            break;
                        case PTRN_HIDE:
                            // Asignar callbacks para el patrón de esconderse
                            sm->launch_effect = hide_pattern_launch;
                            sm->do_effect = hide_pattern_do;
                            sm->finish_effect = hide_pattern_finish;
                            break;
                        case PTRN_OPEN:
                            // Asignar callbacks para el patrón de abrir
                            sm->launch_effect = open_pattern_launch;
                            sm->do_effect = open_pattern_do;
                            sm->finish_effect = open_pattern_finish;
                            break;
                        case PTRN_SLEEP:
                            // Asignar callbacks para el patrón de dormir
                            sm->launch_effect = sleep_pattern_launch;
                            sm->do_effect = sleep_pattern_do;
                            sm->finish_effect = sleep_pattern_finish;
                            break;
                        default:
                            sm->launch_effect = NULL;
                            sm->do_effect = NULL;
                            sm->finish_effect = NULL;
                            break;
                    }
                    
                    // Iniciar el efecto si hay un callback registrado
                    if (sm->launch_effect != NULL) {
                        sm->launch_effect(sm);
                    }
                }
            } else {
                // Solo volver a IDLE si no hay patrón completo
                sm->current_state = SM_STATE_IDLE;
            }
            break;
            
        case SM_STATE_PATTERN_EFFECT:
            // Ejecutar efecto si hay un callback registrado
            if (sm->do_effect != NULL && sm->pattern_system.effect_in_progress) {
                sm->do_effect(sm);
            }
            
            // Incrementar el tiempo de efecto
            sm->effect_time++;
            
            // Forzar la transición después de un tiempo
            if (sm->pattern_system.effect_duration > 100 || sm->effect_time > calc_ticks(MAX_EFFECT_TIME)) {
                sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
                sm->effect_time = 0;
                
                // Llamar al callback de finalización si existe
                if (sm->finish_effect != NULL) {
                    sm->finish_effect(sm);
                }
            }
            break;
            
        case SM_STATE_PATTERN_EFFECT_FINISH:
            // Lógica para finalizar efectos
            sm->timer++;
            if (sm->timer > 20) {
                sm->current_state = SM_STATE_IDLE;
                sm->timer = 0;
                sm->active_pattern = 0;
                sm->is_counter_spell = false;
                
                // Limpiar callbacks y estado del sistema de patrones
                sm->launch_effect = NULL;
                sm->do_effect = NULL;
                sm->finish_effect = NULL;
                sm->pattern_system.effect_in_progress = false;
                sm->pattern_system.effect_type = PTRN_NONE;
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
    // Debug log for enemy state machines
    bool is_enemy = false;
    u16 entity_id = 0;
    
    // Check if this is an enemy state machine
    for (u16 i = 0; i < MAX_ENEMIES; i++) {
        if (sm->entity_id == ENEMY_ENTITY_ID_BASE + i) {
            is_enemy = true;
            entity_id = i;
            break;
        }
    }
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("StateMachine_SendMessage: enemy=%d, type=%d, param=%d, current_state=%d",
                entity_id, type, param, sm->current_state);
    }
    
    Message msg;
    msg.type = type;
    msg.param = param;
    StateMachine_Update(sm, &msg);
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("StateMachine_SendMessage: enemy=%d, new_state=%d after message",
                entity_id, sm->current_state);
    }
}

/**
 * Maneja la finalización de un patrón y la transición al estado de efecto.
 * 
 * @param sm Puntero a la estructura StateMachine
 * @param pattern_id ID del patrón completado
 * @param is_reverse Si el patrón se ejecutó en reversa
 */
void StateMachine_HandlePatternComplete(StateMachine* sm, u16 pattern_id, bool is_reverse) {
    // Debug log for enemy state machines
    bool is_enemy = false;
    u16 entity_id = 0;
    
    // Check if this is an enemy state machine
    for (u16 i = 0; i < MAX_ENEMIES; i++) {
        if (sm->entity_id == ENEMY_ENTITY_ID_BASE + i) {
            is_enemy = true;
            entity_id = i;
            break;
        }
    }
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("StateMachine_HandlePatternComplete: enemy=%d, pattern=%d, is_reverse=%d, current_state=%d",
                entity_id, pattern_id, is_reverse, sm->current_state);
    }
    
    // Actualizar estado
    sm->current_state = SM_STATE_PATTERN_EFFECT;
    sm->active_pattern = pattern_id;
    sm->is_reversed = is_reverse;
    sm->effect_time = 0;
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("  State updated: current_state=%d, active_pattern=%d, is_reversed=%d",
                sm->current_state, sm->active_pattern, sm->is_reversed);
    }
    
    // Obtener el patrón del registro
    Pattern* pattern = get_pattern(pattern_id, sm->owner_type);
    
    if (is_enemy && entity_id == enemy_attacking) {
        kprintf("  Got pattern from registry: %s", pattern ? "SUCCESS" : "FAILED");
    }
    
    if (pattern) {
        // Configurar callbacks desde el patrón
        sm->launch_effect = pattern->launch;
        sm->do_effect = pattern->do_effect;
        sm->finish_effect = pattern->finish;
        
        if (is_enemy && entity_id == enemy_attacking) {
            kprintf("  Callbacks configured: launch=%s, do_effect=%s, finish=%s",
                    sm->launch_effect ? "YES" : "NO",
                    sm->do_effect ? "YES" : "NO",
                    sm->finish_effect ? "YES" : "NO");
        }
        
        // Iniciar el efecto
        if (sm->launch_effect) {
            if (is_enemy && entity_id == enemy_attacking) {
                kprintf("  Launching effect");
            }
            sm->launch_effect(sm);
        }
    }
}

/**
 * Maneja la entrada de una nueva nota en el patrón.
 * 
 * @param sm Puntero a la estructura StateMachine
 * @param note Nota reproducida (1-6)
 */
void StateMachine_HandleNoteInput(StateMachine* sm, u8 note) {
    // Verificar si la nota es válida
    if (note < 1 || note > 6) {
        return;
    }
    
    // Actualizar estado
    sm->current_state = SM_STATE_PLAYING_NOTE;
    sm->current_note = note;
    sm->note_time = 0;
    sm->pattern_system.is_note_playing = true;
    
    // Añadir la nota al patrón actual
    if (sm->note_count < 4) {
        sm->notes[sm->note_count] = note;
        sm->note_count++;
    }
}

/**
 * Maneja el timeout de un patrón en progreso.
 * 
 * @param sm Puntero a la estructura StateMachine
 */
void StateMachine_HandlePatternTimeout(StateMachine* sm) {
    // Resetear el estado del patrón
    sm->note_count = 0;
    sm->pattern_time = 0;
    sm->current_state = SM_STATE_IDLE;
    
    // Limpiar notas
    for (u8 i = 0; i < 4; i++) {
        sm->notes[i] = 0;
    }
}
