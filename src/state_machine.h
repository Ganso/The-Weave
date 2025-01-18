#ifndef _STATE_MACHINE_H_
#define _STATE_MACHINE_H_

#include <genesis.h>
#include "message_types.h"

/**
 * @brief Forward declarations
 */
typedef struct State State;
typedef struct StateMachine StateMachine;

/**
 * @brief State machine structure
 */
struct StateMachine {
    const State* current_state;  // Current state
    const State* default_state;  // Default state to return to
    bool debug_enabled;          // Whether to output debug info
};

/**
 * @brief State handler function type
 * @param sm State machine instance
 * @param msg Message being processed
 * @return Next state or NULL to stay in current state
 */
typedef const State* (*StateHandler)(StateMachine* sm, const Message* msg);

/**
 * @brief State enter/exit function type
 */
typedef void (*StateCallback)(void);

/**
 * @brief State definition structure
 */
struct State {
    const char* name;           // State name for debugging
    StateHandler handler;       // Message handler function
    StateCallback on_enter;     // Called when entering state
    StateCallback on_exit;      // Called when exiting state
};

/**
 * @brief Initialize a state machine
 * @param sm State machine to initialize
 * @param initial_state Starting state
 * @param default_state Default state to return to
 * @param debug Enable debug output
 */
void state_machine_init(StateMachine* sm, const State* initial_state, const State* default_state, bool debug);

/**
 * @brief Send a message to a state machine
 * @param sm State machine to update
 * @param type Message type
 * @param param1 First parameter
 * @param param2 Second parameter
 * @param data Optional data pointer
 * @return true if message was handled
 */
bool sm_send_message(StateMachine* sm, MessageType type, u16 param1, u16 param2, void* data);

/**
 * @brief Update a state machine
 * @param sm State machine to update
 */
void sm_update(StateMachine* sm);

#endif // _STATE_MACHINE_H_
