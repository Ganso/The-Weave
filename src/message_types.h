#ifndef _MESSAGE_TYPES_H_
#define _MESSAGE_TYPES_H_

#include <genesis.h>

/**
 * @brief Message types for system communication
 */
typedef enum {
    MSG_NONE = 0,
    
    // Combat messages
    MSG_COMBAT_START,      // Start combat sequence
    MSG_PATTERN_COMPLETE,  // Pattern cast completed
    MSG_ENEMY_ATTACK,      // Enemy starts casting
    MSG_ENEMY_HIT,        // Enemy takes damage
    MSG_PLAYER_HIT,       // Player takes damage
    MSG_COMBAT_END,       // Combat sequence ending
    
    // Pattern messages
    MSG_PATTERN_START,     // Start pattern casting
    MSG_PATTERN_NOTE,      // Note played in pattern
    MSG_PATTERN_INTERRUPT, // Pattern interrupted
    MSG_PATTERN_EFFECT,    // Pattern effect active
    
    // State messages
    MSG_STATE_CHANGE,      // State machine state changed
    MSG_STATE_UPDATE,      // State machine update tick
    
    // System messages
    MSG_SYSTEM_ERROR,      // System error occurred
    MSG_SYSTEM_DEBUG,      // Debug message
    
    MSG_MAX               // Keep last
} MessageType;

/**
 * @brief Message structure for system communication
 */
typedef struct Message {
    MessageType type;     // Message type
    u16 param1;          // First parameter
    u16 param2;          // Second parameter
    void* data;          // Optional data pointer
} Message;

/**
 * @brief Convert message type to string for debugging
 * @param type Message type to convert
 * @return String representation of message type
 */
const char* msg_type_to_string(MessageType type);

#endif // _MESSAGE_TYPES_H_
