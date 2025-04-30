#include "globals.h"

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
    // Solo actualizar si no estamos en movimiento o si es un estado de efecto
    if (entity->state != STATE_WALKING ||
        state == SM_STATE_PATTERN_EFFECT ||
        state == SM_STATE_PATTERN_EFFECT_FINISH) {
        
        // Actualizar el estado de la entidad
        switch(state) {
            case SM_STATE_IDLE:
                if (entity->state != STATE_WALKING) {
                    entity->state = STATE_IDLE;
                }
                break;
            case SM_STATE_PLAYING_NOTE:
                entity->state = STATE_PLAYING_NOTE;
                break;
            case SM_STATE_PATTERN_CHECK:
                entity->state = STATE_PATTERN_CHECK;
                break;
            case SM_STATE_PATTERN_EFFECT:
                entity->state = STATE_PATTERN_EFFECT;
                break;
            case SM_STATE_PATTERN_EFFECT_FINISH:
                entity->state = STATE_PATTERN_EFFECT_FINISH;
                break;
            default:
                break;
        }
        
        // Actualizar la animación si es un personaje
        if (entity == &obj_character[active_character]) {
            update_character_animation();
        }
    }
}

// Implementación del patrón eléctrico como ejemplo
void electric_pattern_launch(StateMachine* sm) {
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_ELECTRIC, true, true);
    play_pattern_sound(PTRN_ELECTRIC);
    sm->pattern_system.effect_type = PTRN_ELECTRIC;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
}

void electric_pattern_do(StateMachine* sm) {
    // If a counter-spell has succeeded, don't do anything
    if (counter_spell_success) {
        // Reset state machine
        sm->current_state = SM_STATE_IDLE;
        sm->timer = 0;
        sm->effect_time = 0;
        sm->pattern_system.effect_in_progress = false;
        sm->pattern_system.effect_type = PTRN_NONE;
        sm->pattern_system.effect_duration = 0;
        
        // Reset visual effects
        VDP_setHilightShadow(false);
        
        return;
    }
    
    // Efecto visual de trueno
    VDP_setHilightShadow(true);
    SPR_update();
    SYS_doVBlankProcess();
    VDP_setHilightShadow(false);
    SYS_doVBlankProcess();
    SPR_update();
    
    sm->pattern_system.effect_duration++;
    
    // Aplicar efectos de combate si es necesario
    if (is_combat_active && sm->pattern_system.effect_duration == MAX_EFFECT_TIME / 2) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].class_id == ENEMY_CLS_3HEADMONKEY &&
                obj_enemy[nenemy].hitpoints > 0) {
                hit_enemy(nenemy);
            }
        }
    }
}

void electric_pattern_finish(StateMachine* sm) {
    show_pattern_icon(PTRN_ELECTRIC, false, false);
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
}

// Implementación del patrón HIDE (esconderse)
void hide_pattern_launch(StateMachine* sm) {
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_HIDE, true, true);
    play_pattern_sound(PTRN_HIDE);
    sm->pattern_system.effect_type = PTRN_HIDE;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
    movement_active = true;  // Permitir movimiento mientras está escondido
}

void hide_pattern_do(StateMachine* sm) {
    u16 max_effect_time = 400;
    
    // Crear efecto de parpadeo
    if (sm->pattern_system.effect_duration % 2 == 0) {
        show_character(sm->entity_id, true);
    } else {
        show_character(sm->entity_id, false);
    }
    
    sm->pattern_system.effect_duration++;
    
    // Verificar si el efecto ha terminado
    if (sm->pattern_system.effect_duration >= max_effect_time) {
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void hide_pattern_finish(StateMachine* sm) {
    show_pattern_icon(PTRN_HIDE, false, false);
    show_character(sm->entity_id, true);
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
    movement_active = true;  // Permitir que el personaje siga siendo controlable
}

// Implementación del patrón SLEEP (dormir)
void sleep_pattern_launch(StateMachine* sm) {
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_SLEEP, true, true);
    play_pattern_sound(PTRN_SLEEP);
    sm->pattern_system.effect_type = PTRN_SLEEP;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
}

void sleep_pattern_do(StateMachine* sm) {
    // En combate, aplicar efecto de parálisis a los enemigos
    if (is_combat_active && sm->pattern_system.effect_duration == 10) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].hitpoints > 0) {
                // Paralizar al enemigo (no puede atacar por un tiempo)
                obj_enemy[nenemy].paralyzed = true;
                obj_enemy[nenemy].paralyzed_time = 300; // Duración de la parálisis
            }
        }
    }
    
    sm->pattern_system.effect_duration++;
    
    // Efecto visual breve
    if (sm->pattern_system.effect_duration >= 30) {
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void sleep_pattern_finish(StateMachine* sm) {
    show_pattern_icon(PTRN_SLEEP, false, false);
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
}

// Implementación del patrón OPEN (abrir)
void open_pattern_launch(StateMachine* sm) {
    anim_character(sm->entity_id, ANIM_MAGIC);
    show_pattern_icon(PTRN_OPEN, true, true);
    play_pattern_sound(PTRN_OPEN);
    sm->pattern_system.effect_type = PTRN_OPEN;
    sm->pattern_system.effect_in_progress = true;
    sm->pattern_system.effect_duration = 0;
}

void open_pattern_do(StateMachine* sm) {
    // En combate, hacer a los enemigos más vulnerables
    if (is_combat_active && sm->pattern_system.effect_duration == 10) {
        for (u16 nenemy = 0; nenemy < MAX_ENEMIES; nenemy++) {
            if (obj_enemy[nenemy].obj_character.active &&
                obj_enemy[nenemy].hitpoints > 0) {
                // Hacer al enemigo más vulnerable (recibe más daño)
                obj_enemy[nenemy].vulnerable = true;
                obj_enemy[nenemy].vulnerable_time = 300; // Duración de la vulnerabilidad
            }
        }
    }
    
    sm->pattern_system.effect_duration++;
    
    // Efecto visual breve
    if (sm->pattern_system.effect_duration >= 30) {
        sm->current_state = SM_STATE_PATTERN_EFFECT_FINISH;
    }
}

void open_pattern_finish(StateMachine* sm) {
    show_pattern_icon(PTRN_OPEN, false, false);
    sm->pattern_system.effect_type = PTRN_NONE;
    sm->pattern_system.effect_in_progress = false;
    sm->pattern_system.effect_duration = 0;
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
    sm->effect_time = 0;
    sm->entity_id = entity_id;
    
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
                
                // Configurar callbacks según el tipo de patrón
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
    Message msg;
    msg.type = type;
    msg.param = param;
    StateMachine_Update(sm, &msg);
}