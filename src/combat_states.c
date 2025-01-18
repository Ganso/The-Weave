/**
 * @file combat_states.c
 * @brief Implementation of combat state machine
 */

#include "combat_states.h"

// Global reference to current combat state machine instance
static CombatStateMachine* current_csm = NULL;

// Debug output macro - handles both state handlers and enter/exit functions
#define COMBAT_DEBUG(fmt, ...) \
    if (current_csm && current_csm->base.debug_enabled) { \
        kprintf("[COMBAT:%s] " fmt "\n", \
            current_csm->base.current_state ? current_csm->base.current_state->name : "NONE", \
            ##__VA_ARGS__); \
    }

// Forward declarations of state handlers
static const State* handle_idle(StateMachine* sm, const Message* msg);
static const State* handle_player_cast(StateMachine* sm, const Message* msg);
static const State* handle_enemy_cast(StateMachine* sm, const Message* msg);
static const State* handle_pattern_effect(StateMachine* sm, const Message* msg);
static const State* handle_end(StateMachine* sm, const Message* msg);

// State enter/exit handlers
static void on_enter_idle(void);
static void on_exit_idle(void);
static void on_enter_player_cast(void);
static void on_exit_player_cast(void);
static void on_enter_enemy_cast(void);
static void on_exit_enemy_cast(void);
static void on_enter_pattern_effect(void);
static void on_exit_pattern_effect(void);
static void on_enter_end(void);
static void on_exit_end(void);

// State definitions
const State STATE_COMBAT_IDLE = {
    "IDLE",
    handle_idle,
    on_enter_idle,
    on_exit_idle
};

const State STATE_COMBAT_PLAYER_CAST = {
    "PLAYER_CAST",
    handle_player_cast,
    on_enter_player_cast,
    on_exit_player_cast
};

const State STATE_COMBAT_ENEMY_CAST = {
    "ENEMY_CAST",
    handle_enemy_cast,
    on_enter_enemy_cast,
    on_exit_enemy_cast
};

const State STATE_COMBAT_PATTERN_EFFECT = {
    "PATTERN_EFFECT",
    handle_pattern_effect,
    on_enter_pattern_effect,
    on_exit_pattern_effect
};

const State STATE_COMBAT_END = {
    "END",
    handle_end,
    on_enter_end,
    on_exit_end
};

// Helper functions
static void reset_pattern_cast(PatternCastData* cast)
{
    cast->caster_id = 0;
    cast->pattern_id = 0;
    cast->notes_played = 0;
    cast->was_interrupted = false;
    for (u8 i = 0; i < 6; i++) {
        cast->notes[i] = 0;
    }
}

static void reset_combat_state(CombatStateMachine* csm)
{
    csm->is_active = false;
    csm->active_enemy = 0;
    reset_pattern_cast(&csm->player_cast);
    reset_pattern_cast(&csm->enemy_cast);
    csm->effect_timer = 0;
    csm->effect_in_progress = false;
    csm->effect_source = 0;
    csm->effect_pattern = 0;
}

// Public functions
void combat_sm_init(CombatStateMachine* csm, bool debug)
{
    // Set global reference
    current_csm = csm;
    
    // Initialize base state machine
    state_machine_init(&csm->base, &STATE_COMBAT_IDLE, &STATE_COMBAT_IDLE, debug);
    
    // Initialize combat state
    reset_combat_state(csm);
    
    COMBAT_DEBUG("Combat state machine initialized");
}

void combat_sm_update(CombatStateMachine* csm)
{
    // Update global reference
    current_csm = csm;
    sm_update(&csm->base);
}

void combat_sm_start(CombatStateMachine* csm, u16 enemy_id)
{
    if (!csm->is_active) {
        csm->is_active = true;
        csm->active_enemy = enemy_id;
        COMBAT_DEBUG("Starting combat with enemy %d", enemy_id);
        
        // Send combat start message
        msg_send(MSG_COMBAT_START, enemy_id, 0, NULL);
    }
}

void combat_sm_end(CombatStateMachine* csm)
{
    if (csm->is_active) {
        COMBAT_DEBUG("Ending combat");
        
        // Send combat end message
        msg_send(MSG_COMBAT_END, csm->active_enemy, 0, NULL);
        
        // Reset state
        reset_combat_state(csm);
    }
}

bool combat_sm_player_cast_start(CombatStateMachine* csm, u16 player_id, u16 pattern_id)
{
    // Only allow starting cast if in IDLE state
    if (csm->base.current_state == &STATE_COMBAT_IDLE) {
        COMBAT_DEBUG("Player %d starting pattern %d", player_id, pattern_id);
        
        // Initialize cast data
        reset_pattern_cast(&csm->player_cast);
        csm->player_cast.caster_id = player_id;
        csm->player_cast.pattern_id = pattern_id;
        
        // Transition to player cast state
        sm_send_message(&csm->base, MSG_PATTERN_COMPLETE, player_id, pattern_id, NULL);
        return true;
    }
    return false;
}

bool combat_sm_player_cast_note(CombatStateMachine* csm, u16 note_id)
{
    // Only allow adding notes in PLAYER_CAST state
    if (csm->base.current_state == &STATE_COMBAT_PLAYER_CAST) {
        if (csm->player_cast.notes_played < 6) {
            COMBAT_DEBUG("Player adding note %d", note_id);
            
            // Add note to sequence
            csm->player_cast.notes[csm->player_cast.notes_played++] = note_id;
            return true;
        }
    }
    return false;
}

bool combat_sm_enemy_cast_start(CombatStateMachine* csm, u16 enemy_id, u16 pattern_id)
{
    // Only allow starting cast if in IDLE state
    if (csm->base.current_state == &STATE_COMBAT_IDLE) {
        COMBAT_DEBUG("Enemy %d starting pattern %d", enemy_id, pattern_id);
        
        // Initialize cast data
        reset_pattern_cast(&csm->enemy_cast);
        csm->enemy_cast.caster_id = enemy_id;
        csm->enemy_cast.pattern_id = pattern_id;
        
        // Transition to enemy cast state
        sm_send_message(&csm->base, MSG_ENEMY_ATTACK, enemy_id, pattern_id, NULL);
        return true;
    }
    return false;
}

bool combat_sm_enemy_cast_note(CombatStateMachine* csm, u16 note_id)
{
    // Only allow adding notes in ENEMY_CAST state
    if (csm->base.current_state == &STATE_COMBAT_ENEMY_CAST) {
        if (csm->enemy_cast.notes_played < 6) {
            COMBAT_DEBUG("Enemy adding note %d", note_id);
            
            // Add note to sequence
            csm->enemy_cast.notes[csm->enemy_cast.notes_played++] = note_id;
            return true;
        }
    }
    return false;
}

void combat_sm_interrupt_cast(CombatStateMachine* csm, u16 source_id)
{
    // Can interrupt either player or enemy cast
    if (csm->base.current_state == &STATE_COMBAT_PLAYER_CAST) {
        COMBAT_DEBUG("Player cast interrupted by %d", source_id);
        csm->player_cast.was_interrupted = true;
    }
    else if (csm->base.current_state == &STATE_COMBAT_ENEMY_CAST) {
        COMBAT_DEBUG("Enemy cast interrupted by %d", source_id);
        csm->enemy_cast.was_interrupted = true;
    }
}

// State handlers
static const State* handle_idle(StateMachine* sm, const Message* msg)
{
    if (!msg) {
        return NULL;
    }

    switch (msg->type) {
        case MSG_PATTERN_COMPLETE:
            return &STATE_COMBAT_PLAYER_CAST;
        
        case MSG_ENEMY_ATTACK:
            return &STATE_COMBAT_ENEMY_CAST;
        
        case MSG_COMBAT_END:
            return &STATE_COMBAT_END;
            
        default:
            return NULL;
    }
}

static const State* handle_player_cast(StateMachine* sm, const Message* msg)
{
    CombatStateMachine* csm = (CombatStateMachine*)sm;
    
    if (!msg) {
        return NULL;
    }

    switch (msg->type) {
        case MSG_ENEMY_ATTACK:
            // Enemy can interrupt player cast
            combat_sm_interrupt_cast(csm, msg->param1);
            return &STATE_COMBAT_ENEMY_CAST;
        
        case MSG_PATTERN_COMPLETE:
            // Pattern finished successfully
            if (!csm->player_cast.was_interrupted) {
                return &STATE_COMBAT_PATTERN_EFFECT;
            }
            return &STATE_COMBAT_IDLE;
            
        default:
            return NULL;
    }
}

static const State* handle_enemy_cast(StateMachine* sm, const Message* msg)
{
    CombatStateMachine* csm = (CombatStateMachine*)sm;
    
    if (!msg) {
        return NULL;
    }

    switch (msg->type) {
        case MSG_PATTERN_COMPLETE:
            // Player can interrupt enemy cast
            combat_sm_interrupt_cast(csm, msg->param1);
            return &STATE_COMBAT_PLAYER_CAST;
        
        case MSG_ENEMY_ATTACK:
            // Pattern finished successfully
            if (!csm->enemy_cast.was_interrupted) {
                return &STATE_COMBAT_PATTERN_EFFECT;
            }
            return &STATE_COMBAT_IDLE;
            
        default:
            return NULL;
    }
}

static const State* handle_pattern_effect(StateMachine* sm, const Message* msg)
{
    CombatStateMachine* csm = (CombatStateMachine*)sm;
    
    // Update effect timer
    if (csm->effect_in_progress) {
        csm->effect_timer++;
        
        // Check if effect is finished
        if (csm->effect_timer >= 60) { // 1 second effect
            csm->effect_in_progress = false;
            return &STATE_COMBAT_IDLE;
        }
    }
    
    return NULL;
}

static const State* handle_end(StateMachine* sm, const Message* msg)
{
    (void)sm; // Unused parameter
    (void)msg; // Unused parameter
    
    // Just wait for combat to be fully cleaned up
    return NULL;
}

// State enter/exit handlers
static void on_enter_idle(void)
{
    // Nothing to do
    COMBAT_DEBUG("Entering IDLE state");
}

static void on_exit_idle(void)
{
    // Nothing to do
    COMBAT_DEBUG("Exiting IDLE state");
}

static void on_enter_player_cast(void)
{
    COMBAT_DEBUG("Entering PLAYER_CAST state");
    // Show player casting UI
    show_or_hide_enemy_combat_interface(true);
}

static void on_exit_player_cast(void)
{
    COMBAT_DEBUG("Exiting PLAYER_CAST state");
    // Hide casting UI if not going to effect state
    if (current_csm && current_csm->base.current_state != &STATE_COMBAT_PATTERN_EFFECT) {
        show_or_hide_enemy_combat_interface(false);
    }
}

static void on_enter_enemy_cast(void)
{
    COMBAT_DEBUG("Entering ENEMY_CAST state");
    // Show enemy casting UI
    show_or_hide_enemy_combat_interface(true);
}

static void on_exit_enemy_cast(void)
{
    COMBAT_DEBUG("Exiting ENEMY_CAST state");
    // Hide casting UI if not going to effect state
    if (current_csm && current_csm->base.current_state != &STATE_COMBAT_PATTERN_EFFECT) {
        show_or_hide_enemy_combat_interface(false);
    }
}

static void on_enter_pattern_effect(void)
{
    COMBAT_DEBUG("Entering PATTERN_EFFECT state");
    if (!current_csm) return;
    
    // Start effect timer
    current_csm->effect_timer = 0;
    current_csm->effect_in_progress = true;
    
    // Determine effect source
    if (current_csm->player_cast.caster_id != 0) {
        current_csm->effect_source = current_csm->player_cast.caster_id;
        current_csm->effect_pattern = current_csm->player_cast.pattern_id;
        COMBAT_DEBUG("Player pattern %d taking effect", current_csm->effect_pattern);
    } else {
        current_csm->effect_source = current_csm->enemy_cast.caster_id;
        current_csm->effect_pattern = current_csm->enemy_cast.pattern_id;
        COMBAT_DEBUG("Enemy pattern %d taking effect", current_csm->effect_pattern);
    }
}

static void on_exit_pattern_effect(void)
{
    COMBAT_DEBUG("Exiting PATTERN_EFFECT state");
    // Hide all combat UI
    show_or_hide_enemy_combat_interface(false);
}

static void on_enter_end(void)
{
    COMBAT_DEBUG("Entering END state");
    // Clean up combat UI
    show_or_hide_enemy_combat_interface(false);
}

static void on_exit_end(void)
{
    COMBAT_DEBUG("Exiting END state");
    // Nothing to do
}
