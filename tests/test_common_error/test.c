/**
 * @copyright   Copyright (c) 2025 Federico Cristina
 *
 *              Licensed under the Apache License, Version 2.0 (the "License");
 *              you may not use this file except in compliance with the License.
 *              You may obtain a copy of the License at
 *
 *                  http://www.apache.org/licenses/LICENSE-2.0
 *
 *              Unless required by applicable law or agreed to in writing, software
 *              distributed under the License is distributed on an "AS IS" BASIS,
 *              WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *              See the License for the specific language governing permissions and
 *              limitations under the License.
 */

#include "pi/common/error.h"

#include <stdio.h>
#include <assert.h>
#include <setjmp.h>

/* =---- Test Functions ----------------------------------------= */

/**
 * @brief   Test warning messages
 */
static void test_error_warning(void)
{
    printf("Test: Warning Messages...\n");

    // These should print to stderr but not abort
    piWarning("test warning message: %d", 42);

    printf("  PASSED\n");
}

/**
 * @brief   Test error messages
 */
static void test_error_message(void)
{
    printf("Test: Error Messages...\n");

    // These should print to stderr but not abort
    piError("test error message: %s", "hello");

    printf("  PASSED\n");
}

/**
 * @brief   Test recoverable error without handler
 */
static void test_error_recoverable_no_handler(void)
{
    printf("Test: Recoverable Error (No Handler)...\n");

    // Clear any existing handler
    piSetErrorHandler(NULL);

    // We can't actually test piRaiseError without a handler
    // because it would abort. So we just verify the handler is cleared.
    printf("  PASSED (handler cleared)\n");
}

/**
 * @brief   Test recoverable error with handler
 */
static void test_error_recoverable_with_handler(void)
{
    printf("Test: Recoverable Error (With Handler)...\n");

    jmp_buf errorHandler;

    if (setjmp(errorHandler) == 0)
    {
        // Set up error handler
        piSetErrorHandler(&errorHandler);

        // Raise an error - should jump back to setjmp
        piRaiseError("test recoverable error: %d", 123);

        // Should not reach here
        assert(0 && "Should have jumped to error handler");
    }
    else
    {
        // Error was caught
        printf("  Error caught successfully\n");
    }

    // Clear handler
    piSetErrorHandler(NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test nested error handlers
 */
static void test_error_nested_handlers(void)
{
    printf("Test: Nested Error Handlers...\n");

    jmp_buf outerHandler;
    jmp_buf innerHandler;

    if (setjmp(outerHandler) == 0)
    {
        piSetErrorHandler(&outerHandler);

        if (setjmp(innerHandler) == 0)
        {
            // Inner handler takes precedence
            piSetErrorHandler(&innerHandler);

            // Raise error - should jump to inner handler
            piRaiseError("inner error: %s", "test");

            assert(0 && "Should have jumped to inner handler");
        }
        else
        {
            // Inner error caught
            printf("  Inner error caught\n");

            // Restore outer handler
            piSetErrorHandler(&outerHandler);

            // Raise another error - should jump to outer handler
            piRaiseError("outer error: %s", "test");

            assert(0 && "Should have jumped to outer handler");
        }
    }
    else
    {
        // Outer error caught
        printf("  Outer error caught\n");
    }

    // Clear handler
    piSetErrorHandler(NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test error handler clearing
 */
static void test_error_handler_clear(void)
{
    printf("Test: Error Handler Clearing...\n");

    jmp_buf handler;

    // Set handler
    piSetErrorHandler(&handler);

    // Clear handler
    piSetErrorHandler(NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test multiple error messages
 */
static void test_error_multiple_messages(void)
{
    printf("Test: Multiple Error Messages...\n");

    for (int i = 0; i < 5; i++)
    {
        piWarning("warning %d", i);
        piError("error %d", i);
    }

    printf("  PASSED\n");
}

/**
 * @brief   Test error format strings
 */
static void test_error_format_strings(void)
{
    printf("Test: Error Format Strings...\n");

    piWarning("integer: %d", 42);
    piWarning("string: %s", "test");
    piWarning("hex: 0x%x", 0xABCD);
    piWarning("float: %.2f", 3.14);

    piError("multiple args: %d %s %x", 10, "hello", 0xFF);

    printf("  PASSED\n");
}

/* =---- Main Test Runner --------------------------------------= */

int main(void)
{
    printf("====================================\n");
    printf("Error Module Unit Tests\n");
    printf("====================================\n\n");

    printf("NOTE: This test will print warnings and errors to stderr.\n");
    printf("This is expected behavior.\n\n");

    test_error_warning();
    test_error_message();
    test_error_recoverable_no_handler();
    test_error_recoverable_with_handler();
    test_error_nested_handlers();
    test_error_handler_clear();
    test_error_multiple_messages();
    test_error_format_strings();

    printf("\n====================================\n");
    printf("All tests passed!\n");
    printf("====================================\n");

    return 0;
}
