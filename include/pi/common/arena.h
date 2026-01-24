#pragma once

/**
 * @file        arena.h
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
 * @brief       Fast arena allocator for batch memory management.
 */

#ifndef _PI_COMMON_ARENA_H
#define _PI_COMMON_ARENA_H

/* Include common definitions */
#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Arena Allocator ---------------------------------------= */

#ifndef PI_ARENA_DEFAULT_BLOCK_SIZE
/**
 * @brief   Default block size for arena allocations (4KB)
 */
#   define PI_ARENA_DEFAULT_BLOCK_SIZE (4096)
#endif

#ifndef PI_ARENA_ALIGNMENT
/**
 * @brief   Memory alignment for arena allocations
 */
#   define PI_ARENA_ALIGNMENT (sizeof(void*))
#endif

/**
 * +---- ArenaBlock -----------------------+
 */

/**
 * @brief   Represents a single memory block in the arena.
 */
typedef struct _pi_ArenaBlock
{
    /**
     * @brief   Next block in linked list.
     */
    struct _pi_ArenaBlock *next;
    /**
     * @brief   Allocated memory.
     */
    uint8_t               *data;
    /**
     * @brief   Size of this block.
     */
    size_t                 size;
    /**
     * @brief   Bytes used in this block.
     */
    size_t                 used;
} PiArenaBlock;

/**
 * +---- Arena ----------------------------+
 */

/**
 * @brief   Arena allocator for fast batch allocations.
 */
typedef struct _pi_Arena
{
    /**
     * @brief   Current block for allocations.
     */
    PiArenaBlock *curr;
    /**
     * @brief   Head of block list.
     */
    PiArenaBlock *head;
    /**
     * @brief   Default size for new blocks.
     */
    size_t        blockSize;
    /**
     * @brief   Total memory allocated (for stats).
     */
    size_t        totalAllocated;
} PiArena;

/**
 * +---- Arena::Init ----------------------+
 */

/**
 * @brief   Initialize arena with default block size.
 *
 * @param[out] arena       Arena to initialize.
 *
 * @return  Pointer to initialized arena (same as input).
 */
PI_Api(PiArena *) piInitArena(PiArena *const arena);
/**
 * @brief   Initialize arena with custom block size.
 *
 * @param[out] arena       Arena to initialize.
 * @param[in]  blockSize   Size of each memory block.
 *
 * @return  Pointer to initialized arena (same as input).
 */
PI_Api(PiArena *) piInitArenaWithSize(PiArena *const arena, const size_t blockSize);

/**
 * +---- Arena::Free ----------------------+
 */

/**
 * @brief   Free entire arena (all blocks).
 *
 * @param[in,out] arena    Arena to free.
 *
 * @return  NULL (for convenience).
 */
PI_Api(PiArena *) piFreeArena(PiArena *const arena);

/**
 * +---- Arena::Alloc ---------------------+
 */

/**
 * @brief   Allocate memory from arena.
 *
 * @param[in,out] arena    Arena to allocate from.
 * @param[in]     size     Number of bytes to allocate.
 *
 * @return  Pointer to allocated memory (aligned).
 */
PI_Api(void *) piArenaAlloc(PiArena *const arena, const size_t size);
/**
 * @brief   Allocate zero-initialized memory from arena.
 *
 * @param[in,out] arena    Arena to allocate from.
 * @param[in]     size     Number of bytes to allocate.
 *
 * @return  Pointer to zero-initialized memory (aligned).
 */
PI_Api(void *) piArenaAllocZero(PiArena *const arena, const size_t size);

/**
 * +---- Arena::Reset ---------------------+
 */

/**
 * @brief   Reset arena (keep blocks, reset used counters).
 *
 *          Allows reusing arena memory without deallocating blocks.
 *
 * @param[in,out] arena    Arena to reset.
 */
PI_Api(void) piArenaReset(PiArena *const arena);

/**
 * +---- Arena Properties -----------------+
 */

/**
 * @brief   Get total bytes allocated by arena.
 *
 * @param[in] arena        Arena to query.
 *
 * @return  Total bytes allocated across all blocks.
 */
PI_Api(size_t) piArenaGetTotalAllocated(const PiArena *const arena);

/**
 * +---- Arena Helper Macros --------------+
 */

/**
 * @brief   Allocate typed object from arena.
 *
 * @param arena     Arena to allocate from.
 * @param Type      Type of object to allocate.
 *
 * @return  Pointer to allocated object of given type.
 */
#define piArenaAllocType(arena, Type) \
    ((Type*)piArenaAlloc((arena), sizeof(Type)))
/**
 * @brief   Allocate typed array from arena.
 *
 * @param arena     Arena to allocate from.
 * @param Type      Type of array elements.
 * @param count     Number of elements.
 *
 * @return  Pointer to allocated array.
 */
#define piArenaAllocArray(arena, Type, count) \
    ((Type*)piArenaAlloc((arena), sizeof(Type) * (count)))

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
