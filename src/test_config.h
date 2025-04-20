#ifndef _TEST_CONFIG_H_
#define _TEST_CONFIG_H_

#ifdef DEBUG_ON

// Test result structure
typedef struct {
    char* test_name;          // Name of the test (non-const for KLog)
    bool passed;              // Test result
    char* message;           // Error message if failed (non-const for KLog)
} TestResult;

// Enable testing framework
#define TESTING_ENABLED

// Test categories
#define TEST_STATE_MACHINE

// Test configuration
#define TEST_FRAME_DELAY 2      // Frames between tests
#define TEST_CLEANUP_AFTER_EACH TRUE

// Logging macros that use SGDK's KLog
#define TEST_LOG(msg)           KLog(msg)
#define TEST_LOG_U1(msg, p1)    KLog_U1(msg, p1)
#define TEST_LOG_U2(msg, p1, p2) KLog_U2(msg, p1, p2)

// Test assertion macros
#define TEST_ASSERT_MESSAGE(condition, msg) \
    if (!(condition)) { \
        result.passed = FALSE; \
        result.message = msg; \
        return result; \
    }

// Detailed logging for test debugging
#define TEST_LOG_DETAIL(msg) KLog(msg)

#else // DEBUG_ON not defined

// Stub test result structure
typedef struct {
    char* test_name;
    bool passed;
    char* message;
} TestResult;

// Stub out all test macros
#define TESTING_ENABLED
#define TEST_STATE_MACHINE
#define TEST_LOG(msg)
#define TEST_LOG_U1(msg, p1)
#define TEST_LOG_U2(msg, p1, p2)
#define TEST_LOG_DETAIL(msg)
#define TEST_ASSERT_MESSAGE(condition, msg)

#endif // DEBUG_ON

#endif // _TEST_CONFIG_H_