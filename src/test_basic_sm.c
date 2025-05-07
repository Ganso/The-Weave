#include "test_statemachine.h"

#ifdef TESTING_ENABLED
#ifdef TEST_STATE_MACHINE

/**
 * Prueba básica de la máquina de estados
 * Verifica el comportamiento más fundamental:
 * 1. Inicialización correcta
 * 2. Transición simple IDLE -> PLAYING_NOTE
 * 3. Manejo básico de mensajes
 */
TestResult test_basic_state_machine() {
    TestResult result = {
        .test_name = "Basic State Machine Test",
        .passed = TRUE,
        .message = NULL,
        .line_number = 0,
        .file_name = __FILE__
    };
    
    setup_test_environment();
    
    // 1. Verificar estado inicial
    KLog("Checking initial state...\n");
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_IDLE),
                       "Initial state is not IDLE");
    
    // 2. Probar transición básica
    KLog("Testing basic transition...\n");
    Message msg;
    msg.type = MSG_NOTE_PLAYED;
    msg.param = 1;
    StateMachine_Update(&test_sm, &msg);
    
    TEST_ASSERT_MESSAGE(verify_state(&test_sm, SM_STATE_PLAYING_NOTE),
                       "Failed to transition to PLAYING_NOTE");
    
    // 3. Verificar datos del estado
    KLog("Verifying state data...\n");
    TEST_ASSERT_MESSAGE(test_sm.current_note == 1 && test_sm.note_count == 1,
                       "State data not properly updated");
    
    cleanup_test_environment();
    return result;
}

// Añadir la prueba al runner principal
void run_basic_state_machine_tests() {
    KLog("\n=== Running Basic State Machine Tests ===\n");
    
    TestResult result = test_basic_state_machine();
    print_test_result(result);
    
    if (result.passed) {
        KLog("\nBasic state machine functionality verified.\n");
    } else {
        KLog("\nBasic state machine test failed! Check implementation.\n");
        KLog_U2("Failed at %s:%d\n", result.file_name, result.line_number);
    }
    
    KLog("\n=======================================\n");
}

#endif // TEST_STATE_MACHINE
#endif // TESTING_ENABLED
