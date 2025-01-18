#include "globals.h"
#include "message_system.h"

// Debug output macro
#define MSG_DEBUG(fmt, ...) \
    if (DEBUG_STATE_MACHINES) { \
        kprintf("[MSG] " fmt "\n", ##__VA_ARGS__); \
    }

// Message queue
static Message msg_queue_buffer[MAX_QUEUED_MESSAGES];
static u16 msg_queue_read = 0;
static u16 msg_queue_write = 0;
static u16 msg_queue_count = 0;

// Message handlers
static MessageHandler handlers[MAX_MESSAGE_HANDLERS];
static u16 num_handlers = 0;

// Message type strings for debugging
static const char* msg_type_strings[] = {
    "MSG_NONE",
    "MSG_COMBAT_START",
    "MSG_PATTERN_COMPLETE",
    "MSG_ENEMY_ATTACK",
    "MSG_ENEMY_HIT",
    "MSG_PLAYER_HIT",
    "MSG_COMBAT_END",
    "MSG_PATTERN_START",
    "MSG_PATTERN_NOTE",
    "MSG_PATTERN_INTERRUPT",
    "MSG_PATTERN_EFFECT",
    "MSG_STATE_CHANGE",
    "MSG_STATE_UPDATE",
    "MSG_SYSTEM_ERROR",
    "MSG_SYSTEM_DEBUG"
};

const char* msg_type_to_string(MessageType type)
{
    if (type >= 0 && type < MSG_MAX) {
        return msg_type_strings[type];
    }
    return "UNKNOWN";
}

void msg_init(void)
{
    MSG_DEBUG("Initializing message system");
    
    // Clear message queue
    msg_queue_read = 0;
    msg_queue_write = 0;
    msg_queue_count = 0;
    
    // Clear handlers
    num_handlers = 0;
    for (u16 i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        handlers[i] = NULL;
    }
}

bool msg_register_handler(MessageHandler handler)
{
    if (num_handlers >= MAX_MESSAGE_HANDLERS) {
        MSG_DEBUG("Failed to register handler - max handlers reached");
        return false;
    }
    
    handlers[num_handlers++] = handler;
    MSG_DEBUG("Registered message handler %d", num_handlers - 1);
    return true;
}

void msg_unregister_handler(MessageHandler handler)
{
    // Find and remove handler
    for (u16 i = 0; i < num_handlers; i++) {
        if (handlers[i] == handler) {
            // Shift remaining handlers down
            for (u16 j = i; j < num_handlers - 1; j++) {
                handlers[j] = handlers[j + 1];
            }
            handlers[--num_handlers] = NULL;
            MSG_DEBUG("Unregistered message handler %d", i);
            return;
        }
    }
}

bool msg_send(MessageType type, u16 param1, u16 param2, void* data)
{
    MSG_DEBUG("Sending message %s", msg_type_to_string(type));
    
    // Create message
    Message msg;
    msg.type = type;
    msg.param1 = param1;
    msg.param2 = param2;
    msg.data = data;
    
    // Process through handlers
    bool handled = false;
    for (u16 i = 0; i < num_handlers; i++) {
        if (handlers[i] && handlers[i](&msg)) {
            handled = true;
            break;
        }
    }
    
    return handled;
}

bool msg_queue(MessageType type, u16 param1, u16 param2, void* data)
{
    if (msg_queue_count >= MAX_QUEUED_MESSAGES) {
        MSG_DEBUG("Failed to queue message - queue full");
        return false;
    }
    
    MSG_DEBUG("Queueing message %s", msg_type_to_string(type));
    
    // Add message to queue
    Message* msg = &msg_queue_buffer[msg_queue_write];
    msg->type = type;
    msg->param1 = param1;
    msg->param2 = param2;
    msg->data = data;
    
    // Update queue state
    msg_queue_write = (msg_queue_write + 1) % MAX_QUEUED_MESSAGES;
    msg_queue_count++;
    
    return true;
}

u16 msg_update(void)
{
    u16 processed = 0;
    
    // Process queued messages
    while (msg_queue_count > 0) {
        // Get next message
        Message* msg = &msg_queue_buffer[msg_queue_read];
        
        // Process message
        MSG_DEBUG("Processing queued message %s", msg_type_to_string(msg->type));
        msg_send(msg->type, msg->param1, msg->param2, msg->data);
        
        // Update queue state
        msg_queue_read = (msg_queue_read + 1) % MAX_QUEUED_MESSAGES;
        msg_queue_count--;
        processed++;
    }
    
    return processed;
}
