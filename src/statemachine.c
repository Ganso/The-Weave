#include "globals.h"

void StateMachine_Init(StateMachine *sm) {
    sm->current_state = SM_STATE_IDLE;
    sm->timer = 0;
    sm->note_count = 0;
}

void StateMachine_Update(StateMachine *sm, Message *msg) {
    switch (sm->current_state) {
        case SM_STATE_IDLE:
            // Handle messages in the IDLE state
            if (msg != NULL) {
                switch (msg->type) {
                    case MSG_COMBAT_START:
                        sm->current_state = SM_STATE_CASTING;
                        sm->timer = 0;
                        break;
                    default:
                        break;
                }
            }
            break;
        case SM_STATE_CASTING:
            // Handle casting logic
            sm->timer++;
            if (sm->timer > 100) {
                sm->current_state = SM_STATE_EFFECT;
                sm->timer = 0;
            }
            break;
        case SM_STATE_EFFECT:
            // Handle effect logic
            sm->timer++;
            if (sm->timer > 50) {
                sm->current_state = SM_STATE_EFFECT_FINISH;
                sm->timer = 0;
            }
            break;
        case SM_STATE_EFFECT_FINISH:
            // Handle effect finish logic
            sm->timer++;
            if (sm->timer > 20) {
                sm->current_state = SM_STATE_IDLE;
                sm->timer = 0;
            }
            break;
        default:
            // Handle unknown state
            sm->current_state = SM_STATE_IDLE;
            sm->timer = 0;
            break;
    }
}