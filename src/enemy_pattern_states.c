/**
 * @file enemy_pattern_states.c
 * @brief Implementation of enemy pattern state machine
 */

#include "enemy_pattern_states.h"

// Global reference to current enemy pattern state machine instance
EnemyPatternStateMachine* current_epsm = NULL;

// Debug output macro
#define PATTERN_DEBUG(fmt, ...) \
    if (current_epsm && current_epsm->base.debug_enabled) { \
        kprintf("[PATTERN:%s] " fmt "\n", \
            current_epsm->base.current_state ? current_epsm->base.current_state->name : "NONE", \
            ##__VA_ARGS__); \
    }

// Forward declarations of state handlers
static const State* handle_idle(StateMachine* sm, const Message* msg);
static const State* handle_note_sequence(StateMachine* sm, const Message* msg);
static const State* handle_effect_start(StateMachine* sm, const Message* msg);
static const State* handle_effect_active(StateMachine* sm, const Message* msg);
static const State* handle_effect_end(StateMachine* sm, const Message* msg);

// State enter/exit handlers
static void on_enter_idle(void);
static void on_exit_idle(void);
static void on_enter_note_sequence(void);
static void on_exit_note_sequence(void);
static void on_enter_effect_start(void);
static void on_exit_effect_start(void);
static void on_enter_effect_active(void);
static void on_exit_effect_active(void);
static void on_enter_effect_end(void);
static void on_exit_effect_end(void);

// State definitions
const State STATE_ENEMY_IDLE = {
    "IDLE",
    handle_idle,
    on_enter_idle,
    on_exit_idle
};

const State STATE_ENEMY_NOTE_SEQUENCE = {
    "NOTE_SEQUENCE",
    handle_note_sequence,
    on_enter_note_sequence,
    on_exit_note_sequence
};

const State STATE_ENEMY_EFFECT_START = {
    "EFFECT_START",
    handle_effect_start,
    on_enter_effect_start,
    on_exit_effect_start
};

const State STATE_ENEMY_EFFECT_ACTIVE = {
    "EFFECT_ACTIVE",
    handle_effect_active,
    on_enter_effect_active,
    on_exit_effect_active
};

const State STATE_ENEMY_EFFECT_END = {
    "EFFECT_END",
    handle_effect_end,
    on_enter_effect_end,
    on_exit_effect_end
};

// Helper functions
static void reset_enemy_pattern_state(EnemyPatternStateMachine* epsm)
{
    epsm->enemy_id = 0;
    epsm->pattern_id = 0;
    epsm->current_note = 0;
    epsm->note_timer = 0;
    epsm->was_interrupted = false;
    epsm->effect_timer = 0;
    epsm->effect_active = false;
}

// Public functions
void enemy_pattern_sm_init(EnemyPatternStateMachine* epsm, bool debug)
{
    // Set global reference
    current_epsm = epsm;
    
    // Initialize base state machine
    state_machine_init(&epsm->base, &STATE_ENEMY_IDLE, &STATE_ENEMY_IDLE, debug);
    
    // Initialize pattern state
    reset_enemy_pattern_state(epsm);
    
    PATTERN_DEBUG("Enemy pattern state machine initialized");
}

void enemy_pattern_sm_update(EnemyPatternStateMachine* epsm)
{
    // Update global reference
    current_epsm = epsm;
    sm_update(&epsm->base);
}

bool enemy_pattern_sm_start(EnemyPatternStateMachine* epsm, u16 enemy_id, u16 pattern_id)
{
    // Only allow starting pattern if in IDLE state
    if (epsm->base.current_state == &STATE_ENEMY_IDLE) {
        PATTERN_DEBUG("Enemy %d starting pattern %d", enemy_id, pattern_id);
        
        // Initialize pattern data
        reset_enemy_pattern_state(epsm);
        epsm->enemy_id = enemy_id;
        epsm->pattern_id = pattern_id;
        
        // Transition to note sequence state
        sm_send_message(&epsm->base, MSG_ENEMY_ATTACK, enemy_id, pattern_id, NULL);
        return true;
    }
    return false;
}

void enemy_pattern_sm_interrupt(EnemyPatternStateMachine* epsm, u16 source_id)
{
    PATTERN_DEBUG("Pattern interrupted by %d", source_id);
    epsm->was_interrupted = true;
}

bool enemy_pattern_sm_is_active(const EnemyPatternStateMachine* epsm)
{
    return epsm->base.current_state != &STATE_ENEMY_IDLE;
}

// State handlers
static const State* handle_idle(StateMachine* sm, const Message* msg)
{
    if (msg && msg->type == MSG_ENEMY_ATTACK) {
        return &STATE_ENEMY_NOTE_SEQUENCE;
    }
    
    return NULL;
}

static const State* handle_note_sequence(StateMachine* sm, const Message* msg)
{
    EnemyPatternStateMachine* epsm = (EnemyPatternStateMachine*)sm;
    
    if (epsm->was_interrupted) {
        return &STATE_ENEMY_IDLE;
    }
    
    // Update note timer
    epsm->note_timer++;
    
    // Check if current note is finished
    if (epsm->note_timer >= obj_Pattern_Enemy[epsm->pattern_id].recharge_time) {
        // Hide current note
        show_enemy_note(obj_Pattern_Enemy[epsm->pattern_id].notes[epsm->current_note], false, false);
        
        // Move to next note or finish sequence
        if (epsm->current_note < obj_Pattern_Enemy[epsm->pattern_id].numnotes - 1) {
            // Next note
            epsm->current_note++;
            epsm->note_timer = 0;
            show_enemy_note(obj_Pattern_Enemy[epsm->pattern_id].notes[epsm->current_note], true, true);
        } else {
            // Sequence complete
            return &STATE_ENEMY_EFFECT_START;
        }
    }
    
    return NULL;
}

static const State* handle_effect_start(StateMachine* sm, const Message* msg)
{
    EnemyPatternStateMachine* epsm = (EnemyPatternStateMachine*)sm;
    
    if (epsm->was_interrupted) {
        return &STATE_ENEMY_IDLE;
    }
    
    // Initialize pattern-specific effect
    switch (epsm->pattern_id) {
        case PTRN_EN_ELECTIC:
            launch_electric_enemy_pattern();
            break;
        case PTRN_EN_BITE:
            launch_bite_enemy_pattern();
            break;
    }
    
    return &STATE_ENEMY_EFFECT_ACTIVE;
}

static const State* handle_effect_active(StateMachine* sm, const Message* msg)
{
    EnemyPatternStateMachine* epsm = (EnemyPatternStateMachine*)sm;
    
    if (epsm->was_interrupted) {
        return &STATE_ENEMY_EFFECT_END;
    }
    
    // Update effect timer
    epsm->effect_timer++;
    
    // Get max effect time based on pattern
    u16 max_effect_time;
    switch (epsm->pattern_id) {
        case PTRN_EN_ELECTIC:
            max_effect_time = calc_ticks(MAX_EFFECT_TIME_ELECTRIC);
            break;
        case PTRN_EN_BITE:
            max_effect_time = calc_ticks(MAX_EFFECT_TIME_BITE);
            break;
        default:
            max_effect_time = 100;
    }
    
    // Process pattern effect
    if (epsm->effect_timer < max_effect_time) {
        switch (epsm->pattern_id) {
            case PTRN_EN_ELECTIC:
                do_electric_enemy_pattern_effect();
                break;
            case PTRN_EN_BITE:
                do_bite_enemy_pattern_effect();
                break;
        }
    } else {
        return &STATE_ENEMY_EFFECT_END;
    }
    
    return NULL;
}

static const State* handle_effect_end(StateMachine* sm, const Message* msg)
{
    // Clean up effect
    finish_enemy_pattern_effect();
    
    return &STATE_ENEMY_IDLE;
}

// State enter/exit handlers
static void on_enter_idle(void)
{
    PATTERN_DEBUG("Entering IDLE state");
}

static void on_exit_idle(void)
{
    PATTERN_DEBUG("Exiting IDLE state");
}

static void on_enter_note_sequence(void)
{
    PATTERN_DEBUG("Entering NOTE_SEQUENCE state");
    if (!current_epsm) return;
    
    // Show first note
    show_enemy_note(obj_Pattern_Enemy[current_epsm->pattern_id].notes[0], true, true);
}

static void on_exit_note_sequence(void)
{
    PATTERN_DEBUG("Exiting NOTE_SEQUENCE state");
    if (!current_epsm) return;
    
    // Hide current note if interrupted
    if (current_epsm->was_interrupted) {
        show_enemy_note(obj_Pattern_Enemy[current_epsm->pattern_id].notes[current_epsm->current_note], false, false);
    }
}

static void on_enter_effect_start(void)
{
    PATTERN_DEBUG("Entering EFFECT_START state");
    if (!current_epsm) return;
    
    // Initialize effect state
    current_epsm->effect_timer = 0;
    current_epsm->effect_active = true;
}

static void on_exit_effect_start(void)
{
    PATTERN_DEBUG("Exiting EFFECT_START state");
}

static void on_enter_effect_active(void)
{
    PATTERN_DEBUG("Entering EFFECT_ACTIVE state");
}

static void on_exit_effect_active(void)
{
    PATTERN_DEBUG("Exiting EFFECT_ACTIVE state");
}

static void on_enter_effect_end(void)
{
    PATTERN_DEBUG("Entering EFFECT_END state");
    if (!current_epsm) return;
    
    // Clean up effect state
    current_epsm->effect_active = false;
}

static void on_exit_effect_end(void)
{
    PATTERN_DEBUG("Exiting EFFECT_END state");
}
