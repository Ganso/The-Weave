#include "globals.h"
#include "state_machine.h"

// Debug output macro
#define STATE_DEBUG(sm, fmt, ...) \
    if (sm->debug_enabled) { \
        kprintf("[STATE:%s] " fmt "\n", \
            sm->current_state ? sm->current_state->name : "NONE", \
            ##__VA_ARGS__); \
    }

void state_machine_init(StateMachine* sm, const State* initial_state, const State* default_state, bool debug)
{
    sm->current_state = initial_state;
    sm->default_state = default_state;
    sm->debug_enabled = debug;
    
    STATE_DEBUG(sm, "Initialized state machine");
    
    // Call enter callback for initial state
    if (initial_state && initial_state->on_enter) {
        initial_state->on_enter();
    }
}

bool sm_send_message(StateMachine* sm, MessageType type, u16 param1, u16 param2, void* data)
{
    if (!sm->current_state || !sm->current_state->handler) {
        return false;
    }
    
    // Create message
    Message msg;
    msg.type = type;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.data = data;
    
    // Process message through current state
    const State* next_state = sm->current_state->handler(sm, &msg);
    
    // Handle state transition if needed
    if (next_state && next_state != sm->current_state) {
        STATE_DEBUG(sm, "Transitioning to state %s", next_state->name);
        
        // Call exit callback for current state
        if (sm->current_state->on_exit) {
            sm->current_state->on_exit();
        }
        
        // Update current state
        sm->current_state = next_state;
        
        // Call enter callback for new state
        if (next_state->on_enter) {
            next_state->on_enter();
        }
        
        return true;
    }
    
    return false;
}

void sm_update(StateMachine* sm)
{
    // Send update message to current state
    if (sm->current_state) {
        sm_send_message(sm, MSG_STATE_UPDATE, 0, 0, NULL);
    }
}
