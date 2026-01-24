#include "pi/common/arena.h"
#include "pi/common/memory.h"
#include "pi/common/error.h"

#include <string.h>

/* =---- Arena Allocator ---------------------------------------= */

/**
 * +---- ArenaBlock -----------------------+
 */

/**
 * @brief   Allocate a new arena block
 *
 * @param[in] size  Size of the block to allocate
 *
 * @return  Pointer to new block, or NULL if allocation failed
 */
static PiArenaBlock *pi_ArenaBlockAlloc(const size_t size)
{
    PiArenaBlock *const block = PI_New(PiArenaBlock);

    if (PI_UNLIKELY(block == NULL))
        return NULL;

    block->data = (uint8_t *)piMalloc(size);

    if (PI_UNLIKELY(block->data == NULL))
    {
        piFree(block);
        return NULL;
    }

    block->next = NULL;
    block->size = size;
    block->used = 0;

    return block;
}

/**
 * @brief   Free an arena block
 *
 * @param[in] block Block to free
 */
static void pi_ArenaBlockFree(PiArenaBlock *block)
{
    if (block == NULL)
        return;

    piFree(block->data);
    piFree(block);

    return;
}

/**
 * +---- Arena ----------------------------+
 */

/**
 * @brief   Align size to alignment boundary
 *
 * @param[in] size  Size to align
 *
 * @return  Aligned size
 */
static inline size_t pi_ArenaAlignSize(const size_t size)
{
    return (size + (PI_ARENA_ALIGNMENT - 1)) & ~(PI_ARENA_ALIGNMENT - 1);
}

PI_Api(PiArena *) piInitArena(PiArena *const arena)
{
    return piInitArenaWithSize(arena, PI_ARENA_DEFAULT_BLOCK_SIZE);
}

PI_Api(PiArena *) piInitArenaWithSize(PiArena *const arena, const size_t blockSize)
{
    assert(arena != NULL);
    assert(blockSize > 0);

    arena->blockSize = blockSize;
    arena->totalAllocated = 0;
    arena->head = NULL;
    arena->curr = NULL;

    return arena;
}

PI_Api(PiArena *) piFreeArena(PiArena *const arena)
{
    if (arena == NULL)
        return NULL;

    PiArenaBlock *block = arena->head;

    while (block != NULL)
    {
        PiArenaBlock *const next = block->next;

        pi_ArenaBlockFree(block);

        block = next;
    }

    arena->head = NULL;
    arena->curr = NULL;
    arena->totalAllocated = 0;

    return arena;
}

PI_Api(void *) piArenaAlloc(PiArena *const arena, const size_t size)
{
    assert(arena != NULL);
    assert(size > 0);

    const size_t alignedSize = pi_ArenaAlignSize(size);

    /* Check if current block has enough space */
    if ((arena->curr != NULL) && ((arena->curr->used + alignedSize) <= arena->curr->size))
    {
        void *const ptr = arena->curr->data + arena->curr->used;

        arena->curr->used += alignedSize;

        return ptr;
    }

    /* Need new block (make it large enough for this allocation) */
    const size_t newBlockSize =
        (alignedSize > arena->blockSize) ? alignedSize : arena->blockSize;

    PiArenaBlock *const newBlock = pi_ArenaBlockAlloc(newBlockSize);

    if (PI_UNLIKELY(newBlock == NULL))
        piFatal("arena allocation failed: out of memory (%zu bytes requested)", newBlockSize);

    /* Link new block to head of list */
    newBlock->next = arena->head;
    arena->head = newBlock;
    arena->curr = newBlock;
    arena->totalAllocated += newBlockSize;

    /* Allocate from new block */
    void *const ptr = newBlock->data;

    newBlock->used = alignedSize;

    return ptr;
}

PI_Api(void *) piArenaAllocZero(PiArena *const arena, const size_t size)
{
    void *ptr = piArenaAlloc(arena, size);

    /* Set all bytes to zero */
    memset(ptr, 0, size);

    return ptr;
}

PI_Api(void) piArenaReset(PiArena *const arena)
{
    assert(arena != NULL);

    /* Reset used counters but keep blocks allocated */
    PiArenaBlock *block = arena->head;

    while (block != NULL)
    {
        block->used = 0;
        block = block->next;
    }

    /* Reset current block to head */
    arena->curr = arena->head;

    return;
}

PI_Api(size_t) piArenaGetTotalAllocated(const PiArena *const arena)
{
    return (arena != NULL) ? arena->totalAllocated : 0;
}

/* =------------------------------------------------------------= */
