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

#include "pi/common/arena.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* =---- Test Functions ----------------------------------------= */

/**
 * @brief   Test basic allocation
 */
static void test_arena_basic_allocation(void)
{
    printf("Test: Basic Allocation...\n");

    PiArena arena;
    piInitArena(&arena);

    int *a = piArenaAllocType(&arena, int);
    int *b = piArenaAllocType(&arena, int);

    assert(a != NULL);
    assert(b != NULL);
    assert(a != b);

    *a = 42;
    *b = 100;

    assert(*a == 42);
    assert(*b == 100);

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test multiple allocations
 */
static void test_arena_multiple_allocations(void)
{
    printf("Test: Multiple Allocations...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate many small objects
    for (int i = 0; i < 1000; i++)
    {
        int *ptr = piArenaAllocType(&arena, int);
        assert(ptr != NULL);
        *ptr = i;
        assert(*ptr == i);
    }

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test large allocation (larger than block size)
 */
static void test_arena_large_allocation(void)
{
    printf("Test: Large Allocation...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate object larger than default block size (4KB)
    const size_t largeSize = PI_ARENA_DEFAULT_BLOCK_SIZE * 2;
    uint8_t *largeBlock = piArenaAllocArray(&arena, uint8_t, largeSize);

    assert(largeBlock != NULL);

    // Write to it to ensure it's valid
    memset(largeBlock, 0xFF, largeSize);
    assert(largeBlock[0] == 0xFF);
    assert(largeBlock[largeSize - 1] == 0xFF);

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test alignment
 */
static void test_arena_alignment(void)
{
    printf("Test: Alignment...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate various sized objects and check alignment
    for (size_t size = 1; size <= 64; size++)
    {
        void *ptr = piArenaAlloc(&arena, size);
        assert(ptr != NULL);

        // Check alignment
        uintptr_t addr = (uintptr_t)ptr;
        assert((addr % PI_ARENA_ALIGNMENT) == 0);
    }

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test reset functionality
 */
static void test_arena_reset(void)
{
    printf("Test: Reset...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate some memory
    void *p1 = piArenaAlloc(&arena, 100);
    assert(p1 != NULL);

    size_t allocated1 = piArenaGetTotalAllocated(&arena);
    assert(allocated1 > 0);

    // Reset arena
    piArenaReset(&arena);

    // Allocate again - should reuse memory
    void *p2 = piArenaAlloc(&arena, 100);
    assert(p2 != NULL);

    size_t allocated2 = piArenaGetTotalAllocated(&arena);

    // Total allocated should be the same (blocks reused)
    assert(allocated1 == allocated2);

    // Pointers might be the same (reused the same block)
    // This is implementation-dependent, so we don't assert it

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test statistics tracking
 */
static void test_arena_statistics(void)
{
    printf("Test: Statistics...\n");

    PiArena arena;
    piInitArena(&arena);

    assert(piArenaGetTotalAllocated(&arena) == 0);

    // Allocate some memory (should trigger first block allocation)
    piArenaAlloc(&arena, 100);
    size_t allocated1 = piArenaGetTotalAllocated(&arena);
    assert(allocated1 >= PI_ARENA_DEFAULT_BLOCK_SIZE);

    // Allocate more within same block
    piArenaAlloc(&arena, 100);
    size_t allocated2 = piArenaGetTotalAllocated(&arena);
    assert(allocated1 == allocated2); // Should still be same block

    // Allocate large enough to trigger new block
    piArenaAlloc(&arena, PI_ARENA_DEFAULT_BLOCK_SIZE);
    size_t allocated3 = piArenaGetTotalAllocated(&arena);
    assert(allocated3 > allocated2); // Should have allocated new block

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test typed array allocation macro
 */
static void test_arena_array_allocation(void)
{
    printf("Test: Array Allocation...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate array using macro
    int *array = piArenaAllocArray(&arena, int, 10);
    assert(array != NULL);

    // Write to array
    for (int i = 0; i < 10; i++)
    {
        array[i] = i * 10;
    }

    // Verify
    for (int i = 0; i < 10; i++)
    {
        assert(array[i] == i * 10);
    }

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test zero initialization
 */
static void test_arena_zero_init(void)
{
    printf("Test: Zero Initialization...\n");

    PiArena arena;
    piInitArena(&arena);

    // Allocate zero-initialized memory
    const size_t size = 128;
    uint8_t *mem = piArenaAllocZero(&arena, size);
    assert(mem != NULL);

    // Verify all bytes are zero
    for (size_t i = 0; i < size; i++)
    {
        assert(mem[i] == 0);
    }

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test multiple blocks
 */
static void test_arena_multiple_blocks(void)
{
    printf("Test: Multiple Blocks...\n");

    // Use small block size to force multiple blocks
    PiArena arena;
    piInitArenaWithSize(&arena, 128);

    size_t lastAllocated = 0;
    int blockCount = 0;

    // Allocate until we have at least 3 blocks
    for (int i = 0; i < 100; i++)
    {
        piArenaAlloc(&arena, 64);

        size_t currentAllocated = piArenaGetTotalAllocated(&arena);
        if (currentAllocated > lastAllocated)
        {
            blockCount++;
            lastAllocated = currentAllocated;
        }

        if (blockCount >= 3)
            break;
    }

    assert(blockCount >= 3);

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/**
 * @brief   Test custom block size
 */
static void test_arena_custom_block_size(void)
{
    printf("Test: Custom Block Size...\n");

    const size_t customBlockSize = 8192; // 8KB
    PiArena arena;
    piInitArenaWithSize(&arena, customBlockSize);

    // Allocate to trigger block allocation
    piArenaAlloc(&arena, 100);

    size_t allocated = piArenaGetTotalAllocated(&arena);
    assert(allocated == customBlockSize);

    piFreeArena(&arena);

    printf("  PASSED\n");
}

/* =---- Main Test Runner --------------------------------------= */

int main(void)
{
    printf("====================================\n");
    printf("Arena Allocator Unit Tests\n");
    printf("====================================\n\n");

    test_arena_basic_allocation();
    test_arena_multiple_allocations();
    test_arena_large_allocation();
    test_arena_alignment();
    test_arena_reset();
    test_arena_statistics();
    test_arena_array_allocation();
    test_arena_zero_init();
    test_arena_multiple_blocks();
    test_arena_custom_block_size();

    printf("\n====================================\n");
    printf("All tests passed!\n");
    printf("====================================\n");

    return 0;
}
