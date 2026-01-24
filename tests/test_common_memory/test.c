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

#include "pi/common/memory.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

/* =---- Test Functions ----------------------------------------= */

/**
 * @brief   Test basic malloc allocation
 */
static void test_memory_malloc(void)
{
    printf("Test: Malloc...\n");

    int *ptr = (int *)piMalloc(sizeof(int));
    assert(ptr != NULL);

    *ptr = 42;
    assert(*ptr == 42);

    piFree(ptr);

    printf("  PASSED\n");
}

/**
 * @brief   Test calloc zero-initialization
 */
static void test_memory_calloc(void)
{
    printf("Test: Calloc...\n");

    int *array = (int *)piCalloc(10, sizeof(int));
    assert(array != NULL);

    // All elements should be zero
    for (int i = 0; i < 10; i++)
    {
        assert(array[i] == 0);
    }

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

    piFree(array);

    printf("  PASSED\n");
}

/**
 * @brief   Test realloc
 */
static void test_memory_realloc(void)
{
    printf("Test: Realloc...\n");

    int *ptr = (int *)piMalloc(sizeof(int));
    assert(ptr != NULL);

    *ptr = 100;

    // Grow allocation
    ptr = (int *)piRealloc(ptr, 10 * sizeof(int));
    assert(ptr != NULL);

    // Original value should be preserved
    assert(ptr[0] == 100);

    // Write to expanded memory
    for (int i = 1; i < 10; i++)
    {
        ptr[i] = i;
    }

    // Verify
    for (int i = 1; i < 10; i++)
    {
        assert(ptr[i] == i);
    }

    piFree(ptr);

    printf("  PASSED\n");
}

/**
 * @brief   Test piResize function
 */
static void test_memory_resize(void)
{
    printf("Test: Resize...\n");

    // Start with NULL
    int *ptr = NULL;

    // Allocate (oldSize=0, newSize>0)
    ptr = (int *)piResize(ptr, 0, 5 * sizeof(int));
    assert(ptr != NULL);

    for (int i = 0; i < 5; i++)
    {
        ptr[i] = i;
    }

    // Grow (oldSize>0, newSize>oldSize)
    ptr = (int *)piResize(ptr, 5 * sizeof(int), 10 * sizeof(int));
    assert(ptr != NULL);

    // Original values should be preserved
    for (int i = 0; i < 5; i++)
    {
        assert(ptr[i] == i);
    }

    // Shrink (oldSize>0, newSize<oldSize)
    ptr = (int *)piResize(ptr, 10 * sizeof(int), 3 * sizeof(int));
    assert(ptr != NULL);

    // First 3 values should still be there
    for (int i = 0; i < 3; i++)
    {
        assert(ptr[i] == i);
    }

    // Free (oldSize>0, newSize=0)
    ptr = (int *)piResize(ptr, 3 * sizeof(int), 0);
    assert(ptr == NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test piFree with NULL
 */
static void test_memory_free_null(void)
{
    printf("Test: Free NULL...\n");

    // Should be safe to free NULL
    piFree(NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_New macro
 */
static void test_memory_new_macro(void)
{
    printf("Test: PI_New Macro...\n");

    typedef struct
    {
        int x;
        int y;
    } Point;

    Point *pt = PI_New(Point);
    assert(pt != NULL);

    pt->x = 10;
    pt->y = 20;

    assert(pt->x == 10);
    assert(pt->y == 20);

    piFree(pt);

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_NewArray macro
 */
static void test_memory_new_array_macro(void)
{
    printf("Test: PI_NewArray Macro...\n");

    int *array = PI_NewArray(int, 20);
    assert(array != NULL);

    // Should be zero-initialized
    for (int i = 0; i < 20; i++)
    {
        assert(array[i] == 0);
    }

    // Write
    for (int i = 0; i < 20; i++)
    {
        array[i] = i * 2;
    }

    // Verify
    for (int i = 0; i < 20; i++)
    {
        assert(array[i] == i * 2);
    }

    piFree(array);

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_Resize macro
 */
static void test_memory_resize_macro(void)
{
    printf("Test: PI_Resize Macro...\n");

    int *array = NULL;

    // Allocate
    array = PI_Resize(int, array, 0, 5);
    assert(array != NULL);

    for (int i = 0; i < 5; i++)
    {
        array[i] = i;
    }

    // Grow
    array = PI_Resize(int, array, 5, 10);
    assert(array != NULL);

    for (int i = 0; i < 5; i++)
    {
        assert(array[i] == i);
    }

    // Free
    array = PI_Resize(int, array, 10, 0);
    assert(array == NULL);

    printf("  PASSED\n");
}

/**
 * @brief   Test aligned allocation
 */
static void test_memory_aligned_allocation(void)
{
    printf("Test: Aligned Allocation...\n");

    // Allocate cache-line aligned memory
    void *ptr = piMallocAligned(1024, 64);
    assert(ptr != NULL);

    // Check alignment
    uintptr_t addr = (uintptr_t)ptr;
    assert((addr % 64) == 0);

    // Write to it
    memset(ptr, 0xAA, 1024);

    piFreeAligned(ptr);

    printf("  PASSED\n");
}

/**
 * @brief   Test various alignment values
 */
static void test_memory_various_alignments(void)
{
    printf("Test: Various Alignments...\n");

    size_t alignments[] = {8, 16, 32, 64, 128, 256};
    size_t numAlignments = PI_CountOf(alignments);

    for (size_t i = 0; i < numAlignments; i++)
    {
        size_t alignment = alignments[i];
        void *ptr = piMallocAligned(512, alignment);
        assert(ptr != NULL);

        uintptr_t addr = (uintptr_t)ptr;
        assert((addr % alignment) == 0);

        piFreeAligned(ptr);
    }

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_ShouldGrow macro
 */
static void test_memory_should_grow(void)
{
    printf("Test: PI_ShouldGrow Macro...\n");

    // Should grow when capacity is insufficient
    assert(PI_ShouldGrow(10, 8, 3) == true);   // 8 + 3 > 10

    // Should not grow when capacity is sufficient
    assert(PI_ShouldGrow(10, 5, 3) == false);  // 5 + 3 <= 10
    assert(PI_ShouldGrow(10, 7, 3) == false);  // 7 + 3 = 10

    // Edge cases
    assert(PI_ShouldGrow(0, 0, 1) == true);    // Empty array
    assert(PI_ShouldGrow(10, 10, 1) == true);  // Already full

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_BitsOf macro
 */
static void test_memory_bits_of(void)
{
    printf("Test: PI_BitsOf Macro...\n");

    assert(PI_BitsOf(char) == 8);
    assert(PI_BitsOf(int) >= 16);     // At least 16 bits
    assert(PI_BitsOf(long) >= 32);    // At least 32 bits

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_CountOf macro
 */
static void test_memory_count_of(void)
{
    printf("Test: PI_CountOf Macro...\n");

    int array1[10];
    assert(PI_CountOf(array1) == 10);

    char array2[256];
    assert(PI_CountOf(array2) == 256);

    double array3[] = {1.0, 2.0, 3.0};
    assert(PI_CountOf(array3) == 3);

    printf("  PASSED\n");
}

/**
 * @brief   Test PI_HasFlag macro
 */
static void test_memory_has_flag(void)
{
    printf("Test: PI_HasFlag Macro...\n");

    #define FLAG_READ   0x01
    #define FLAG_WRITE  0x02
    #define FLAG_EXEC   0x04

    int flags = FLAG_READ | FLAG_WRITE;

    assert(PI_HasFlag(flags, FLAG_READ) == true);
    assert(PI_HasFlag(flags, FLAG_WRITE) == true);
    assert(PI_HasFlag(flags, FLAG_EXEC) == false);

    printf("  PASSED\n");
}

/**
 * @brief   Test large allocations
 */
static void test_memory_large_allocation(void)
{
    printf("Test: Large Allocation...\n");

    // Allocate 1 MB
    const size_t size = 1024 * 1024;
    char *buffer = (char *)piMalloc(size);
    assert(buffer != NULL);

    // Write pattern
    memset(buffer, 0x55, size);

    // Verify
    assert(buffer[0] == 0x55);
    assert(buffer[size - 1] == 0x55);

    piFree(buffer);

    printf("  PASSED\n");
}

/* =---- Main Test Runner --------------------------------------= */

int main(void)
{
    printf("====================================\n");
    printf("Memory Module Unit Tests\n");
    printf("====================================\n\n");

    test_memory_malloc();
    test_memory_calloc();
    test_memory_realloc();
    test_memory_resize();
    test_memory_free_null();
    test_memory_new_macro();
    test_memory_new_array_macro();
    test_memory_resize_macro();
    test_memory_aligned_allocation();
    test_memory_various_alignments();
    test_memory_should_grow();
    test_memory_bits_of();
    test_memory_count_of();
    test_memory_has_flag();
    test_memory_large_allocation();

    printf("\n====================================\n");
    printf("All tests passed!\n");
    printf("====================================\n");

    return 0;
}
