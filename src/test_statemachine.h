#ifndef _TEST_STATEMACHINE_H_
#define _TEST_STATEMACHINE_H_

#ifdef TESTING_ENABLED
#ifdef TEST_STATE_MACHINE

// Test functions
TestResult test_timing_behavior();    // Prueba de tiempos y timeouts
TestResult test_state_sequence();     // Prueba de secuencia de estados
TestResult test_message_handling();   // Prueba de manejo de mensajes
TestResult test_additional_features(); // Prueba de características adicionales

// Helper functions
void simulate_time_passage(u16 frames);
bool verify_state(StateMachine *sm, SM_State expected_state);
void play_note_sequence(StateMachine *sm, u8 *notes, u8 count);
bool verify_effects(StateMachine *sm, u16 expected_pattern);

// Test environment
void setup_test_environment();
void cleanup_test_environment();
void print_test_result(TestResult result);

// Main test runner
void run_state_machine_tests();

#else
// Stub function when tests are disabled
static inline void run_state_machine_tests() {
    KLog("State machine tests disabled\n");
}
#endif // TEST_STATE_MACHINE

#else
// Stub function when all testing is disabled
static inline void run_state_machine_tests() {}
#endif // TESTING_ENABLED

#endif // _TEST_STATEMACHINE_H_