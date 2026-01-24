#pragma once

/**
 * @file        memory.h
 *
 * @author      Federico Crisitina <federico.cristina@outlook.it>
 * 
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
 *
 * @brief       Safe memory allocation wrappers with error checking and optional tracking.
 */

#ifndef _PI_COMMON_MEMORY_H
#define _PI_COMMON_MEMORY_H

/* Common definitions (and <stdlib.h> inclusion) */
#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Memory Utility Macros ---------------------------------= */

#ifndef PI_BitsOf
/**
 * @brief   Calculates the bit width of a type or value.
 *
 * @param x  Type name or expression.
 *
 * @return  Number of bits in the type.
 *
 * @example
 * @code
 *     size_t intBits = PI_BitsOf(int);         // 32 on most platforms
 *     size_t ptrBits = PI_BitsOf(void*);       // 64 on 64-bit systems
 *     size_t varBits = PI_BitsOf(myVariable);  // Bits of myVariable's type
 * @endcode
 */
#   define PI_BitsOf(x) (sizeof(x) * CHAR_BIT)
#endif

#ifndef PI_CountOf
/**
 * @brief   Calculates the number of elements in a statically allocated array.
 *
 * @param array  Array name (must be a true array, not a pointer).
 *
 * @return  Number of elements in the array.
 *
 * @warning Only works with arrays, not pointers. Produces incorrect results
 *          if used with a pointer or dynamically allocated memory.
 *
 * @example
 * @code
 *     int values[] = {1, 2, 3, 4, 5};
 *     size_t count = PI_CountOf(values);  // Returns 5
 * @endcode
 */
#   define PI_CountOf(array) (sizeof(array) / sizeof(*(array)))
#endif

#ifndef PI_HasFlag
/**
 * @brief   Checks if specific flags are set in a bitfield.
 *
 * @param x     Value to test.
 * @param flag  Flag bits to check for.
 *
 * @return  Non-zero if all bits in flag are set in x, zero otherwise.
 *
 * @example
 * @code
 *     #define FLAG_READONLY  0x01
 *     #define FLAG_HIDDEN    0x02
 *
 *     int attrs = FLAG_READONLY | FLAG_HIDDEN;
 * 
 *     if (PI_HasFlag(attrs, FLAG_READONLY)) {
 *         // File is read-only
 *     }
 * @endcode
 */
#   define PI_HasFlag(x, flag) \
        (((x) & (flag)) != 0)
#endif

#ifndef PI_ShouldGrow
/**
 * @brief   Determines if a container needs to grow to accommodate more elements.
 *
 * @param cap    Current capacity of the container.
 * @param count  Current number of elements in the container.
 * @param n      Number of new elements to add.
 *
 * @return  Non-zero if capacity is insufficient, zero if space is available.
 *
 * @note    Uses overflow-safe arithmetic to prevent integer overflow bugs.
 *
 * @example
 * @code
 *     if (PI_ShouldGrow(capacity, count, 5)) {
 *         capacity = capacity * 2 + 5;
 *         array = realloc(array, capacity * sizeof(*array));
 *     }
 * @endcode
 */
#   define PI_ShouldGrow(cap, count, n) \
        ((n) > (cap) - (count))
#endif

/* =---- Memory Allocation Functions ---------------------------= */

/**
 * @brief   Allocates a block of uninitialized memory.
 *
 * @param size  Number of bytes to allocate. Must be greater than 0.
 *
 * @return  Pointer to the allocated memory block.
 *
 * @note    The allocated memory is not initialized and may contain garbage values.
 *          If allocation fails, prints a fatal error and terminates the program.
 *          Memory must be freed with piFree() or free().
 *
 * @see     piCalloc(), piNew(), piFree()
 */
PI_Api(void *) piMalloc(const size_t size);
/**
 * @brief   Allocates memory aligned to a specific boundary.
 *
 * @param size       Number of bytes to allocate. Must be greater than 0.
 * @param alignment  Alignment boundary in bytes. Must be a power of 2.
 *
 * @return  Pointer to the allocated memory, aligned to the specified boundary.
 *
 * @note    Uses platform-specific aligned allocation:
 *          - C11: aligned_alloc() (when available)
 *          - Windows: _aligned_malloc()
 *          - POSIX: posix_memalign()
 *
 *          If allocation fails, prints a fatal error and terminates.
 *          Memory must be freed with piFreeAligned().
 *
 * @warning Do NOT use piFree() or free() on memory allocated with this function.
 *          Use piFreeAligned() instead.
 *
 * @example
 * @code
 *     // Allocate cache-line aligned memory for performance
 *     void *buffer = piMallocAligned(4096, 64);
 *     // ... use buffer ...
 *     piFreeAligned(buffer);
 * @endcode
 *
 * @see     piFreeAligned(), PI_CACHE_LINE_SIZE
 */
PI_Api(void *) piMallocAligned(const size_t size, const size_t alignment);
/**
 * @brief   Allocates a zero-initialized array.
 *
 * @param count  Number of elements to allocate.
 * @param size   Size of each element in bytes.
 *
 * @return  Pointer to the allocated and zero-initialized memory block.
 *
 * @note    All bytes in the allocated memory are set to zero.
 *          If allocation fails, prints a fatal error and terminates the program.
 *          Memory must be freed with piFree() or free().
 *
 * @example
 * @code
 *     int *array = piCalloc(100, sizeof(int));  // Allocates 100 ints, all zero
 * @endcode
 *
 * @see     piMalloc(), piNewArray(), piFree()
 */
PI_Api(void *) piCalloc(const size_t count, const size_t size);

/**
 * @brief   Reallocates a memory block to a new size.
 *
 * @param block    Pointer to previously allocated memory block, or NULL.
 * @param newSize  New size for the memory block in bytes.
 *
 * @return  Pointer to the reallocated memory block.
 *
 * @note    If block is NULL, behaves like piMalloc(newSize).
 *          If newSize is 0, behavior is undefined (use piResize() instead).
 *          The contents of the block are preserved up to min(oldSize, newSize).
 *          If reallocation fails, prints a fatal error and terminates.
 *
 * @warning The returned pointer may differ from the input pointer.
 *          Do not use the old pointer after calling this function.
 *
 * @see     piResize(), piMalloc(), piFree()
 */
PI_Api(void *) piRealloc(void *const block, const size_t newSize);
/**
 * @brief   Intelligently resizes a memory block, handling edge cases.
 *
 * @param block    Pointer to previously allocated memory block, or NULL.
 * @param oldSize  Current size of the block in bytes (0 if block is NULL).
 * @param newSize  Desired new size in bytes (0 to free the block).
 *
 * @return  Pointer to the resized memory block, or NULL if newSize is 0.
 *
 * @note    Handles all combinations of NULL/non-NULL pointers and zero/non-zero sizes:
 *          - oldSize == 0, newSize > 0:  Allocates new block (like piMalloc)
 *          - oldSize > 0,  newSize > 0:  Resizes block (like piRealloc)
 *          - oldSize > 0,  newSize == 0: Frees block and returns NULL
 *          - oldSize == 0, newSize == 0: Returns NULL
 *
 * @example
 * @code
 *     char *buf = NULL;
 *     buf = piResize(buf, 0, 100);      // Allocate
 *     buf = piResize(buf, 100, 200);    // Grow
 *     buf = piResize(buf, 200, 50);     // Shrink
 *     buf = piResize(buf, 50, 0);       // Free (returns NULL)
 * @endcode
 *
 * @see     piRealloc(), PI_Resize()
 */
PI_Api(void *) piResize(void *const block, const size_t oldSize, const size_t newSize);

/**
 * @brief   Frees a memory block allocated by piMalloc(), piCalloc(), or piRealloc().
 *
 * @param ptr  Pointer to memory block to free, or NULL.
 *
 * @note    If ptr is NULL, this function has no effect (safe to call).
 *          After freeing, the pointer becomes invalid and must not be used.
 *
 * @see     piMalloc(), piCalloc(), piRealloc()
 */
PI_Api(void) piFree(void *ptr);
/**
 * @brief   Frees memory allocated by piMallocAligned().
 *
 * @param ptr  Pointer to aligned memory block to free, or NULL.
 *
 * @note    If ptr is NULL, this function has no effect (safe to call).
 *          Uses platform-specific deallocation matching the allocation method.
 *
 * @warning Only use this function for memory allocated with piMallocAligned().
 *
 * @see     piMallocAligned()
 */
PI_Api(void) piFreeAligned(void *ptr);

/* =---- Memory Allocation Macros ------------------------------= */

#ifndef PI_New
/**
 * @brief   Allocates memory for a single instance of a type.
 *
 * @param Type  Type name to allocate (e.g., int, struct MyStruct).
 *
 * @return  Pointer to uninitialized memory for one instance of Type.
 *
 * @example
 * @code
 *     typedef struct { int x, y; } Point;
 *     Point *pt = PI_New(Point);
 *     pt->x = 10;
 *     pt->y = 20;
 *     piFree(pt);
 * @endcode
 *
 * @see     piMalloc(), PI_NewArray(), piFree()
 */
#   define PI_New(Type) \
        ((Type *)piMalloc(sizeof(Type)))
#endif

#ifndef PI_NewArray
/**
 * @brief   Allocates zero-initialized memory for an array of a type.
 *
 * @param Type   Type name for array elements.
 * @param count  Number of elements to allocate.
 *
 * @return  Pointer to zero-initialized array of count elements of Type.
 *
 * @example
 * @code
 *     int *numbers = PI_NewArray(int, 100);    // Allocates 100 ints, all zero
 *     numbers[0] = 42;
 *     piFree(numbers);
 * @endcode
 *
 * @see     piCalloc(), PI_New(), piFree()
 */
#   define PI_NewArray(Type, count) \
        ((Type *)piCalloc((count), sizeof(Type)))
#endif

#ifndef PI_Resize
/**
 * @brief   Type-safe wrapper for piResize() that handles sizing automatically.
 *
 * @param Type     Type name of the array elements.
 * @param block    Pointer to the memory block to resize.
 * @param oldSize  Current number of elements (not bytes).
 * @param newSize  Desired number of elements (not bytes).
 *
 * @return  Pointer to the resized array, cast to Type*.
 *
 * @example
 * @code
 *     int *array = NULL;
 *     array = PI_Resize(int, array, 0, 10);    // Allocate 10 ints
 *     array = PI_Resize(int, array, 10, 20);   // Resize to 20 ints
 *     array = PI_Resize(int, array, 20, 0);    // Free (returns NULL)
 * @endcode
 *
 * @see     piResize()
 */
#   define PI_Resize(Type, block, oldSize, newSize) \
        ((Type *)piResize((void *)(block), sizeof(Type) * (oldSize), sizeof(Type) * (newSize)))
#endif

/* =---- Memory Tracking (Optional) ----------------------------= */

#ifdef PI_ENABLE_MEMORY_TRACKING
/**
 * @brief   Returns the total number of bytes currently allocated.
 *
 * @return  Total allocated memory in bytes (excludes freed memory).
 *
 * @note    Only available when PI_ENABLE_MEMORY_TRACKING is defined.
 *          Useful for debugging memory leaks and monitoring memory usage.
 */
PI_Api(size_t) piGetAllocatedMemory(void);

/**
 * @brief   Returns the peak memory usage since program start.
 *
 * @return  Maximum memory allocated at any point in bytes.
 *
 * @note    Only available when PI_ENABLE_MEMORY_TRACKING is defined.
 */
PI_Api(size_t) piGetPeakMemory(void);

/**
 * @brief   Returns the total number of allocation calls made.
 *
 * @return  Count of piMalloc(), piCalloc(), and piRealloc() calls.
 *
 * @note    Only available when PI_ENABLE_MEMORY_TRACKING is defined.
 */
PI_Api(size_t) piGetAllocationCount(void);

/**
 * @brief   Prints a summary of memory usage statistics to stderr.
 *
 * @note    Only available when PI_ENABLE_MEMORY_TRACKING is defined.
 *          Useful for debugging and performance analysis.
 */
PI_Api(void) piPrintMemoryStats(void);
#endif

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
