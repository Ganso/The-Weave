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