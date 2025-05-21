#ifndef _TEST_H_
#define _TEST_H_

/**
 * Test Framework for The Weave
 *
 * This header unifies all test-related includes and provides
 * common test functionality. All test files should include
 * this header instead of individual test headers.
 */

// Include globals.h which already has all necessary includes
#include "globals.h"

// Forward declarations
struct _test_result;

// Test result structure
typedef struct _test_result {
    const char* test_name;    // Name of the test
    bool passed;              // Test result
    const char* message;      // Error message if failed
    u16 line_number;         // Line where test failed
    const char* file_name;   // File where test failed
} TestResult;

// Test assertion macros
#define TEST_ASSERT(condition) \
    if (!(condition)) { \
        result.passed = FALSE; \
        result.message = #condition; \
        result.line_number = __LINE__; \
        result.file_name = __FILE__; \
        return result; \
    }

#define TEST_ASSERT_MESSAGE(condition, msg) \
    if (!(condition)) { \
        result.passed = FALSE; \
        result.message = (msg); \
        result.line_number = __LINE__; \
        result.file_name = __FILE__; \
        return result; \
    }

// Test utilities
void setup_test_environment();
void cleanup_test_environment();
void print_test_result(TestResult result);

// Main test runners
void run_state_machine_tests();
void run_basic_state_machine_tests();

#endif // _TEST_H_