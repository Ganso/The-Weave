#ifndef STATEMACHINE_H
#define STATEMACHINE_H

typedef enum {
    SM_STATE_IDLE,
    SM_STATE_CASTING,
    SM_STATE_EFFECT,
    SM_STATE_EFFECT_FINISH
} SM_State;

typedef enum {
    MSG_PATTERN_COMPLETE,
    MSG_COMBAT_START,
    MSG_ENEMY_DEFEATED
} MessageType;

typedef struct {
    MessageType type;
    u16 param;
} Message;

typedef struct {
    SM_State current_state;
    u16 timer;
    u8 notes[4];
    u8 note_count;
} StateMachine;

void StateMachine_Init(StateMachine *sm);
void StateMachine_Update(StateMachine *sm, Message *msg);

#endif