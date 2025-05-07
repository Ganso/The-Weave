#include "globals.h"

#ifdef TESTING_ENABLED
#ifdef TEST_STATE_MACHINE

// Global test state
static StateMachine test_sm;
static bool test_environment_ready = FALSE;

// Test messages
static char MSG_TIMING_BEHAVIOR[] = "Timing Behavior";
static char MSG_STATE_SEQUENCE[] = "State Sequence";
static char MSG_MESSAGE_HANDLING[] = "Message Handling";
static char MSG_ADDITIONAL_FEATURES[] = "Additional Features";

static char MSG_NOTE_EARLY[] = "Note finished too early";
static char MSG_NOTE_TIMEOUT[] = "Note did not finish after max time";
static char MSG_EFFECT_EARLY[] = "Effect finished too early";
static char MSG_EFFECT_TIMEOUT[] = "Effect did not finish after max time";

// Setup and cleanup
void setup_test_environment() {
    if (!test_environment_ready) {
        StateMachine_Init(&test_sm, 0);
        test_environment_ready = TRUE;
    }
}

void cleanup_test_environment() {
    if (test_environment_ready) {
        StateMachine_Init(&test_sm, 0);
        test_environment_ready = FALSE;
    }
}

// Helper functions
void simulate_time_passage(u16 frames) {
    while (frames > 0) {
        StateMachine_Update(&test_sm, NULL);
        frames--;
    }
}

bool verify_state(StateMachine *sm, SM_State expected_state) {
    return sm->current_state == expected_state;
}

void play_note_sequence(StateMachine *sm, u8 *notes, u8 count) {
    for (u8 i = 0; i < count; i++) {
        Message msg;
        msg.type = MSG_NOTE_PLAYED;
        msg.param = notes[i];
        StateMachine_Update(sm, &msg);
        simulate_time_passage(1);
    }
}

bool verify_effects(StateMachine *sm, u16 expected_pattern) {
    return sm->active_pattern == expected_pattern;
}

// Test result printing
void print_test_result(TestResult result) {
    if (result.passed) {
        TEST_LOG("✓ PASS: ");
        TEST_LOG(result.test_name);
        TEST_LOG("\n");
    } else {
        TEST_LOG("✗ FAIL: ");
        TEST_LOG(result.test_name);
        TEST_LOG("\n");
        if (result.message) {
            TEST_LOG_DETAIL("  Error: ");
            TEST_LOG_DETAIL(result.message);
            TEST_LOG_DETAIL("\n");
        }
    }
}

// Individual test implementations
TestResult test_timing_behavior() {
    TestResult result = {
        .test_name = MSG_TIMING_BEHAVIOR,
        .passed = TRUE,
        .message = NULL
    };
    
    setup_test_environment();
    
    // 1. Verificar transición por tiempo de nota
    TEST_LOG("Testing note playing timeout...\n");
    Message msg;
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    
    // Simular tiempo justo antes del timeout
    simulate_time_passage(MAX_NOTE_PLAYING_TIME - 1);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PLAYING_NOTE),
                       MSG_NOTE_EARLY);
    
    // Simular tiempo para provocar timeout
    simulate_time_passage(2);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_CHECK),
                       MSG_NOTE_TIMEOUT);
    
    // 2. Verificar tiempo de efecto
    TEST_LOG("Testing pattern effect timing...\n");
    msg.type = MSG_PATTERN_COMPLETE;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    
    simulate_time_passage(MAX_EFFECT_TIME - 1);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_EFFECT),
                       MSG_EFFECT_EARLY);
    
    simulate_time_passage(2);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_EFFECT_FINISH),
                       MSG_EFFECT_TIMEOUT);
    
    cleanup_test_environment();
    return result;
}

TestResult test_state_sequence() {
    TestResult result = {
        .test_name = MSG_STATE_SEQUENCE,
        .passed = TRUE,
        .message = NULL
    };
    
    setup_test_environment();
    
    static char MSG_INITIAL_STATE[] = "Initial state is not IDLE";
    static char MSG_PLAYING_NOTE[] = "Failed to transition to PLAYING_NOTE";
    static char MSG_PATTERN_CHECK[] = "Failed to transition to PATTERN_CHECK";
    static char MSG_PATTERN_EFFECT[] = "Failed to transition to PATTERN_EFFECT";
    static char MSG_EFFECT_FINISH[] = "Failed to transition to PATTERN_EFFECT_FINISH";
    static char MSG_BACK_IDLE[] = "Failed to transition back to IDLE";
    
    // 1. Inicio en IDLE
    TEST_LOG("Checking initial state...\n");
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_IDLE),
                       MSG_INITIAL_STATE);
    
    // 2. IDLE -> PLAYING_NOTE
    TEST_LOG("Testing IDLE -> PLAYING_NOTE...\n");
    Message msg;
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PLAYING_NOTE),
                       MSG_PLAYING_NOTE);
    
    // 3. PLAYING_NOTE -> PATTERN_CHECK
    TEST_LOG("Testing PLAYING_NOTE -> PATTERN_CHECK...\n");
    simulate_time_passage(MAX_NOTE_PLAYING_TIME + 1);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_CHECK),
                       MSG_PATTERN_CHECK);
    
    // 4. PATTERN_CHECK -> PATTERN_EFFECT
    TEST_LOG("Testing PATTERN_CHECK -> PATTERN_EFFECT...\n");
    msg.type = MSG_PATTERN_COMPLETE;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_EFFECT),
                       MSG_PATTERN_EFFECT);
    
    // 5. PATTERN_EFFECT -> PATTERN_EFFECT_FINISH
    TEST_LOG("Testing PATTERN_EFFECT -> PATTERN_EFFECT_FINISH...\n");
    simulate_time_passage(MAX_EFFECT_TIME + 1);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_EFFECT_FINISH),
                       MSG_EFFECT_FINISH);
    
    // 6. PATTERN_EFFECT_FINISH -> IDLE
    TEST_LOG("Testing PATTERN_EFFECT_FINISH -> IDLE...\n");
    simulate_time_passage(21);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_IDLE),
                       MSG_BACK_IDLE);
    
    cleanup_test_environment();
    return result;
}

TestResult test_message_handling() {
    TestResult result = {
        .test_name = MSG_MESSAGE_HANDLING,
        .passed = TRUE,
        .message = NULL
    };
    
    setup_test_environment();
    
    static char MSG_NOTE_FAILED[] = "Failed to handle MSG_NOTE_PLAYED";
    static char MSG_NOTE_DATA[] = "Note data not properly stored";
    static char MSG_PATTERN_FAILED[] = "Failed to handle MSG_PATTERN_COMPLETE";
    static char MSG_PATTERN_DATA[] = "Pattern ID not properly stored";
    static char MSG_TIMEOUT_FAILED[] = "Failed to handle MSG_PATTERN_TIMEOUT";
    
    // 1. MSG_NOTE_PLAYED
    TEST_LOG("Testing MSG_NOTE_PLAYED handling...\n");
    Message msg;
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 42;
    StateMachine_Update(&test_sm, &msg);
    
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PLAYING_NOTE),
                       MSG_NOTE_FAILED);
    TEST_ASSERT_MESSAGE(test_sm.current_note == 42 && test_sm.notes[0] == 42,
                       MSG_NOTE_DATA);
    
    // 2. MSG_PATTERN_COMPLETE
    TEST_LOG("Testing MSG_PATTERN_COMPLETE handling...\n");
    simulate_time_passage(MAX_NOTE_PLAYING_TIME + 1);
    
    msg.type = MSG_PATTERN_COMPLETE;
    msg.param = 7;
    StateMachine_Update(&test_sm, &msg);
    
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PATTERN_EFFECT),
                       MSG_PATTERN_FAILED);
    TEST_ASSERT_MESSAGE(test_sm.active_pattern == 7,
                       MSG_PATTERN_DATA);
    
    // 3. MSG_PATTERN_TIMEOUT
    TEST_LOG("Testing MSG_PATTERN_TIMEOUT handling...\n");
    setup_test_environment();
    
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    
    // Verificar que el timeout ocurre naturalmente
    simulate_time_passage(MAX_PATTERN_WAIT_TIME + 1);
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_IDLE),
                       MSG_TIMEOUT_FAILED);
    
    cleanup_test_environment();
    return result;
}

TestResult test_additional_features() {
    TestResult result = {
        .test_name = MSG_ADDITIONAL_FEATURES,
        .passed = TRUE,
        .message = NULL
    };
    
    setup_test_environment();
    
    static char MSG_REVERSED[] = "Reversed pattern not handled correctly";
    static char MSG_PATTERN_TIME[] = "Pattern time not tracked correctly";
    static char MSG_COMBAT[] = "Combat start/end not handled correctly";
    
    // 1. Patrón invertido
    TEST_LOG("Testing reversed pattern...\n");
    Message msg;
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 1;
    // Solo verificamos que podemos marcar un patrón como invertido
    test_sm.is_reversed = TRUE;
    StateMachine_Update(&test_sm, &msg);
    
    TEST_ASSERT_MESSAGE(test_sm.is_reversed,
                       MSG_REVERSED);
    
    // 2. Tiempo entre notas
    TEST_LOG("Testing pattern timing...\n");
    simulate_time_passage(1);
    TEST_ASSERT_MESSAGE(test_sm.pattern_time == 1,
                       MSG_PATTERN_TIME);
    
    // 3. Mensajes de combate
    TEST_LOG("Testing combat messages...\n");
    setup_test_environment();
    
    msg.type = MSG_COMBAT_START;
    StateMachine_Update(&test_sm, &msg);
    
    msg.type = MSG_COMBAT_END;
    StateMachine_Update(&test_sm, &msg);
    
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_IDLE),
                       MSG_COMBAT);
    
    cleanup_test_environment();
    return result;
}

// Main test runner
void run_state_machine_tests() {
    TEST_LOG("\n=== Running State Machine Tests ===\n");
    
    TestResult results[] = {
        test_timing_behavior(),
        test_state_sequence(),
        test_message_handling(),
        test_additional_features()
    };
    
    u8 total_tests = sizeof(results) / sizeof(TestResult);
    u8 passed_tests = 0;
    
    for (u8 i = 0; i < total_tests; i++) {
        print_test_result(results[i]);
        if (results[i].passed) passed_tests++;
    }
    
    static char summary[32];
    sprintf(summary, "%d/%d passed\n", passed_tests, total_tests);
    TEST_LOG("\nTest Summary: ");
    TEST_LOG(summary);
    TEST_LOG("============================\n");
}

#endif // TEST_STATE_MACHINE
#endif // TESTING_ENABLED
