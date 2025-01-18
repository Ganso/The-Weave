#ifndef _MESSAGE_SYSTEM_H_
#define _MESSAGE_SYSTEM_H_

#include "message_types.h"

// Maximum number of queued messages
#define MAX_QUEUED_MESSAGES 32

// Maximum number of message handlers
#define MAX_MESSAGE_HANDLERS 16

/**
 * @brief Message handler function type
 * @param msg Pointer to message being handled
 * @return true if message was handled, false to continue processing
 */
typedef bool (*MessageHandler)(const Message* msg);

/**
 * @brief Initialize message system
 */
void msg_init(void);

/**
 * @brief Register a message handler
 * @param handler Function to handle messages
 * @return true if handler was registered
 */
bool msg_register_handler(MessageHandler handler);

/**
 * @brief Unregister a message handler
 * @param handler Function to remove
 */
void msg_unregister_handler(MessageHandler handler);

/**
 * @brief Send a message immediately
 * @param type Message type
 * @param param1 First parameter
 * @param param2 Second parameter
 * @param data Optional data pointer
 * @return true if any handler processed the message
 */
bool msg_send(MessageType type, u16 param1, u16 param2, void* data);

/**
 * @brief Queue a message for later processing
 * @param type Message type
 * @param param1 First parameter
 * @param param2 Second parameter
 * @param data Optional data pointer
 * @return true if message was queued
 */
bool msg_queue(MessageType type, u16 param1, u16 param2, void* data);

/**
 * @brief Process queued messages
 * @return Number of messages processed
 */
u16 msg_update(void);

#endif // _MESSAGE_SYSTEM_H_
