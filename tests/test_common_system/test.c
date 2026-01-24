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

#include "pi/common/system.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* =---- Test Functions ----------------------------------------= */

/**
 * @brief   Test system initialization and cleanup
 */
static void test_system_init_free(void)
{
    printf("Test: System Init/Free...\n");

    piInitSystem();

    // System should be initialized
    const char *cwd = piGetCurrentDirectory();
    assert(cwd != NULL);
    assert(strlen(cwd) > 0);

    piFreeSystem();

    printf("  PASSED\n");
}

/**
 * @brief   Test page size retrieval
 */
static void test_system_page_size(void)
{
    printf("Test: Page Size...\n");

    piInitSystem();

    size_t pageSize = piGetPageSize();

    // Page size should be a power of 2 and reasonable value
    assert(pageSize > 0);
    assert(pageSize >= 512);     // At least 512 bytes
    assert(pageSize <= 65536);   // At most 64 KB

    // Check if power of 2
    assert((pageSize & (pageSize - 1)) == 0);

    piFreeSystem();

    printf("  PASSED\n");
}

/**
 * @brief   Test current directory retrieval
 */
static void test_system_current_directory(void)
{
    printf("Test: Current Directory...\n");

    piInitSystem();

    const char *cwd = piGetCurrentDirectory();
    assert(cwd != NULL);
    assert(strlen(cwd) > 0);

    // Should contain some path characters
#ifdef _WIN32
    // Windows paths have backslashes or drive letters
    bool hasPathChar = (strchr(cwd, '\\') != NULL || strchr(cwd, ':') != NULL);
#else
    // Unix paths start with /
    bool hasPathChar = (cwd[0] == '/');
#endif
    assert(hasPathChar);

    piFreeSystem();

    printf("  PASSED\n");
}

/**
 * @brief   Test virtual terminal support (Windows-specific)
 */
static void test_system_virtual_terminal(void)
{
    printf("Test: Virtual Terminal...\n");

    piInitSystem();

    // Should not crash
    piEnableVirtualTerminal();

    piFreeSystem();

    printf("  PASSED\n");
}

/**
 * @brief   Test console title setting
 */
static void test_system_console_title(void)
{
    printf("Test: Console Title...\n");

    piInitSystem();

    // Should not crash
    piSetConsoleTitle("Pi Test - System");

    piFreeSystem();

    printf("  PASSED\n");
}

/**
 * @brief   Test multiple init/free cycles
 */
static void test_system_multiple_init(void)
{
    printf("Test: Multiple Init/Free Cycles...\n");

    for (int i = 0; i < 3; i++)
    {
        piInitSystem();

        const char *cwd = piGetCurrentDirectory();
        assert(cwd != NULL);

        piFreeSystem();
    }

    printf("  PASSED\n");
}

/**
 * @brief   Test idempotent initialization
 */
static void test_system_idempotent_init(void)
{
    printf("Test: Idempotent Init...\n");

    piInitSystem();

    const char *cwd1 = piGetCurrentDirectory();
    assert(cwd1 != NULL);

    // Second init should be safe
    piInitSystem();

    const char *cwd2 = piGetCurrentDirectory();
    assert(cwd2 != NULL);

    // Should be the same pointer (not re-initialized)
    assert(cwd1 == cwd2);

    piFreeSystem();

    printf("  PASSED\n");
}

/* =---- Main Test Runner --------------------------------------= */

int main(void)
{
    printf("====================================\n");
    printf("System Module Unit Tests\n");
    printf("====================================\n\n");

    test_system_init_free();
    test_system_page_size();
    test_system_current_directory();
    test_system_virtual_terminal();
    test_system_console_title();
    test_system_multiple_init();
    test_system_idempotent_init();

    printf("\n====================================\n");
    printf("All tests passed!\n");
    printf("====================================\n");

    return 0;
}
