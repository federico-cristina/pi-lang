#include "pi/runtime/env.h"

#include <stdarg.h>
#include <stdio.h>

#include <string.h>

/* =---- Platform Detection ----------------------------------------= */

#ifdef _WIN32
#   define PI_PLATFORM_WINDOWS 1
#   include <Windows.h>
#else
#   define PI_PLATFORM_POSIX 1
#   include <sys/mman.h>
#   include <unistd.h>
#   include <stdint.h>
#endif

/* =---- Heap Configuration ----------------------------------------= */

/**
 * @brief   Alignment for all heap allocations (must be power of 2).
 */
#define PI_HEAP_ALIGNMENT 16
/**
 * @brief   Minimum allocation block size.
 */
#define PI_HEAP_MIN_BLOCK_SIZE 32

/**
 * @brief   Block header flags.
 */
#define PI_BLOCK_FREE 0x00
#define PI_BLOCK_USED 0x01

/* =---- Heap Data Structures --------------------------------------= */

#if PI_PLATFORM_POSIX

/**
 * @brief   Block header for the free-list allocator.
 *
 * Each allocated or free block in the heap has this header.
 * The header contains metadata for allocation tracking and free-list management.
 */
typedef struct PiBlockHeader
{
    size_t               size;      /**< Size of block data (excluding header). */
    uint8_t              flags;     /**< PI_BLOCK_FREE or PI_BLOCK_USED. */
    uint8_t              padding[7];/**< Padding for alignment. */
    struct PiBlockHeader *next;     /**< Next block in memory (for coalescing). */
    struct PiBlockHeader *nextFree; /**< Next free block in free list. */
} PiBlockHeader;

/**
 * @brief   POSIX heap structure using mmap.
 *
 * This structure manages a memory region allocated via mmap,
 * using a first-fit free-list allocator.
 */
typedef struct PiPosixHeap
{
    void          *base;       /**< Base address of mmap'd region. */
    size_t         totalSize;  /**< Total size of the heap in bytes. */
    size_t         usedSize;   /**< Currently used size (excluding headers). */
    PiBlockHeader *firstBlock; /**< First block in memory. */
    PiBlockHeader *freeList;   /**< Head of free list. */
} PiPosixHeap;

#endif /* PI_PLATFORM_POSIX */

/* =---- Internal Helper Functions ---------------------------------= */

/**
 * @brief   Align a size up to the specified alignment boundary.
 *
 * @param[in] size      Size to align.
 * @param[in] alignment Alignment boundary (must be power of 2).
 *
 * @return  Aligned size.
 */
static inline size_t pi_AlignUp(size_t size, size_t alignment)
{
    return (size + alignment - 1) & ~(alignment - 1);
}

/* =---- POSIX Heap Implementation ---------------------------------= */

#if PI_PLATFORM_POSIX

/**
 * @brief   Create a new POSIX heap using mmap.
 *
 * @param[in] initialSize Initial heap size (0 for default).
 *
 * @return  Pointer to heap structure, or NULL on failure.
 */
static PiPosixHeap *pi_PosixHeapCreate(size_t initialSize)
{
    /* Get system page size */
    const size_t pageSize = (size_t)sysconf(_SC_PAGESIZE);

    /* Use default size if not specified */
    if (initialSize == 0)
        initialSize = PI_ENV_HEAP_DEFAULT_SIZE;

    /* Enforce minimum size */
    if (initialSize < PI_ENV_HEAP_MIN_SIZE)
        initialSize = PI_ENV_HEAP_MIN_SIZE;

    /* Align to page boundary */
    initialSize = pi_AlignUp(initialSize, pageSize);

    /* Allocate heap structure with standard malloc */
    PiPosixHeap *heap = (PiPosixHeap *)malloc(sizeof(PiPosixHeap));

    if (!heap)
        return NULL;

    /* Map the heap memory region */
    void *base = mmap(
        NULL,
        initialSize,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (base == MAP_FAILED)
    {
        free(heap);
        return NULL;
    }

    /* Initialize heap structure */
    heap->base = base;
    heap->totalSize = initialSize;
    heap->usedSize = 0;

    /* Initialize first block (entire heap is one free block) */
    PiBlockHeader *firstBlock = (PiBlockHeader *)base;

    firstBlock->size = initialSize - sizeof(PiBlockHeader);
    firstBlock->flags = PI_BLOCK_FREE;
    firstBlock->next = NULL;
    firstBlock->nextFree = NULL;

    heap->firstBlock = firstBlock;
    heap->freeList = firstBlock;

    return heap;
}

/**
 * @brief   Destroy a POSIX heap, releasing all memory.
 *
 * @param[in] heap Heap to destroy.
 */
static void pi_PosixHeapDestroy(PiPosixHeap *heap)
{
    if (!heap)
        return;

    if (heap->base)
        munmap(heap->base, heap->totalSize);

    free(heap);

    return;
}

/**
 * @brief   Allocate memory from a POSIX heap using first-fit strategy.
 *
 * @param[in] heap Heap to allocate from.
 * @param[in] size Number of bytes to allocate.
 *
 * @return  Pointer to allocated memory, or NULL on failure.
 */
static void *pi_PosixHeapAlloc(PiPosixHeap *heap, size_t size)
{
    if (!heap || size == 0)
        return NULL;

    /* Align the requested size */
    size = pi_AlignUp(size, PI_HEAP_ALIGNMENT);

    if (size < PI_HEAP_MIN_BLOCK_SIZE)
        size = PI_HEAP_MIN_BLOCK_SIZE;

    /* First-fit: find a free block large enough */
    PiBlockHeader *prev = NULL;
    PiBlockHeader *block = heap->freeList;

    while (block)
    {
        if (block->size >= size)
        {
            /* Found a suitable block */
            const size_t remaining = block->size - size;

            /* Split if there's enough room for another block */
            if (remaining >= sizeof(PiBlockHeader) + PI_HEAP_MIN_BLOCK_SIZE)
            {
                /* Create new free block after this one */
                PiBlockHeader *newBlock = (PiBlockHeader *)((uint8_t *)block + sizeof(PiBlockHeader) + size);

                newBlock->size = remaining - sizeof(PiBlockHeader);
                newBlock->flags = PI_BLOCK_FREE;
                newBlock->next = block->next;
                newBlock->nextFree = block->nextFree;

                block->size = size;
                block->next = newBlock;

                /* Update free list */
                if (prev)
                    prev->nextFree = newBlock;
                else
                    heap->freeList = newBlock;
            }
            else
            {
                /* Use entire block */
                if (prev)
                    prev->nextFree = block->nextFree;
                else
                    heap->freeList = block->nextFree;
            }

            /* Mark as used */
            block->flags = PI_BLOCK_USED;
            block->nextFree = NULL;
            heap->usedSize += block->size;

            /* Return pointer to data area (after header) */
            void *result = (void *)((uint8_t *)block + sizeof(PiBlockHeader));

            /* Zero the allocated memory */
            memset(result, 0, block->size);

            return result;
        }

        prev = block;
        block = block->nextFree;
    }

    /* No suitable block found */
    return NULL;
}

/**
 * @brief   Free memory back to a POSIX heap with coalescing.
 *
 * @param[in] heap Heap the memory belongs to.
 * @param[in] ptr  Pointer to memory to free.
 */
static void pi_PosixHeapFree(PiPosixHeap *heap, void *ptr)
{
    if (!heap || !ptr)
        return;

    /* Get block header */
    PiBlockHeader *block = (PiBlockHeader *)((uint8_t *)ptr - sizeof(PiBlockHeader));

    if (block->flags != PI_BLOCK_USED)
        return; /* Already free or corrupted */

    /* Mark as free */
    block->flags = PI_BLOCK_FREE;
    heap->usedSize -= block->size;

    /* Try to coalesce with next block if free */
    if (block->next && block->next->flags == PI_BLOCK_FREE)
    {
        PiBlockHeader *nextBlock = block->next;

        /* Remove next block from free list */
        PiBlockHeader *prevFree = NULL;
        PiBlockHeader *curr = heap->freeList;

        while (curr && curr != nextBlock)
        {
            prevFree = curr;
            curr = curr->nextFree;
        }

        if (curr == nextBlock)
        {
            if (prevFree)
                prevFree->nextFree = nextBlock->nextFree;
            else
                heap->freeList = nextBlock->nextFree;
        }

        /* Merge blocks */
        block->size += sizeof(PiBlockHeader) + nextBlock->size;
        block->next = nextBlock->next;
    }

    /* Add block to free list (front insertion) */
    block->nextFree = heap->freeList;
    heap->freeList = block;

    return;
}

/**
 * @brief   Reallocate memory in a POSIX heap.
 *
 * @param[in] heap    Heap the memory belongs to.
 * @param[in] ptr     Pointer to existing memory.
 * @param[in] newSize New size in bytes.
 *
 * @return  Pointer to reallocated memory, or NULL on failure.
 */
static void *pi_PosixHeapRealloc(PiPosixHeap *heap, void *ptr, size_t newSize)
{
    if (!heap)
        return NULL;

    if (!ptr)
        return pi_PosixHeapAlloc(heap, newSize);

    if (newSize == 0)
    {
        pi_PosixHeapFree(heap, ptr);
        return NULL;
    }

    /* Get block header */
    PiBlockHeader *block = (PiBlockHeader *)((uint8_t *)ptr - sizeof(PiBlockHeader));

    /* Align new size */
    newSize = pi_AlignUp(newSize, PI_HEAP_ALIGNMENT);

    if (newSize < PI_HEAP_MIN_BLOCK_SIZE)
        newSize = PI_HEAP_MIN_BLOCK_SIZE;

    /* Current block is large enough */
    if (block->size >= newSize)
        return ptr;

    /* Try to expand into next block if free */
    if (block->next && block->next->flags == PI_BLOCK_FREE)
    {
        const size_t combinedSize = block->size + sizeof(PiBlockHeader) + block->next->size;

        if (combinedSize >= newSize)
        {
            PiBlockHeader *nextBlock = block->next;

            /* Remove next block from free list */
            PiBlockHeader *prevFree = NULL;
            PiBlockHeader *curr = heap->freeList;

            while (curr && curr != nextBlock)
            {
                prevFree = curr;
                curr = curr->nextFree;
            }

            if (curr == nextBlock)
            {
                if (prevFree)
                    prevFree->nextFree = nextBlock->nextFree;
                else
                    heap->freeList = nextBlock->nextFree;
            }

            /* Merge blocks */
            heap->usedSize -= block->size;
            block->size = combinedSize;
            block->next = nextBlock->next;
            heap->usedSize += block->size;

            return ptr;
        }
    }

    /* Allocate new block and copy */
    void *newPtr = pi_PosixHeapAlloc(heap, newSize);

    if (!newPtr)
        return NULL;

    memcpy(newPtr, ptr, block->size);
    pi_PosixHeapFree(heap, ptr);

    return newPtr;
}

#endif /* PI_PLATFORM_POSIX */

/* =---- Memory Handler Functions ----------------------------------= */

#if PI_PLATFORM_WINDOWS

/**
 * @brief   Windows heap allocation handler.
 */
static void *pi_WindowsAllocHandler(PiEnv *const env, size_t size)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    void *result = HeapAlloc((HANDLE)env->heap, HEAP_ZERO_MEMORY, (SIZE_T)size);

    if (!result)
        piEnvRaiseError(env, "out of memory (cannot allocate %zu bytes)", size);

    return result;
}

/**
 * @brief   Windows heap reallocation handler.
 */
static void *pi_WindowsReallocHandler(PiEnv *const env, void *block, size_t newSize)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    if (!block)
        return pi_WindowsAllocHandler(env, newSize);

    if (newSize == 0)
    {
        HeapFree((HANDLE)env->heap, 0, block);
        return NULL;
    }

    void *result = HeapReAlloc((HANDLE)env->heap, HEAP_ZERO_MEMORY, block, (SIZE_T)newSize);

    if (!result)
        piEnvRaiseError(env, "out of memory (cannot reallocate to %zu bytes)", newSize);

    return result;
}

/**
 * @brief   Windows heap free handler.
 */
static void pi_WindowsFreeHandler(PiEnv *const env, void *block)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    if (block)
        HeapFree((HANDLE)env->heap, 0, block);

    return;
}

#else /* PI_PLATFORM_POSIX */

/**
 * @brief   POSIX heap allocation handler.
 */
static void *pi_PosixAllocHandler(PiEnv *const env, size_t size)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    void *result = pi_PosixHeapAlloc((PiPosixHeap *)env->heap, size);

    if (!result)
        piEnvRaiseError(env, "out of memory (cannot allocate %zu bytes)", size);

    return result;
}

/**
 * @brief   POSIX heap reallocation handler.
 */
static void *pi_PosixReallocHandler(PiEnv *const env, void *block, size_t newSize)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    void *result = pi_PosixHeapRealloc((PiPosixHeap *)env->heap, block, newSize);

    if (!result && newSize > 0)
        piEnvRaiseError(env, "out of memory (cannot reallocate to %zu bytes)", newSize);

    return result;
}

/**
 * @brief   POSIX heap free handler.
 */
static void pi_PosixFreeHandler(PiEnv *const env, void *block)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    pi_PosixHeapFree((PiPosixHeap *)env->heap, block);

    return;
}

#endif /* PI_PLATFORM_POSIX */

/* =---- GC Context Initialization ---------------------------------= */

/**
 * @brief   Initialize the garbage collection context.
 *
 * @param[out] gc GC context to initialize.
 */
static void pi_InitGcContext(PiGcContext *gc)
{
    assert(gc != NULL);

    /* Object lists */
    gc->objectList = NULL;
    gc->grayList = NULL;

    /* Root sets */
    gc->vmStack = NULL;
    gc->globalRoots = NULL;
    gc->globalRootCount = 0;
    gc->globalRootCap = 0;

    /* Configuration defaults */
    gc->config.nextGcThreshold = 1024 * 1024; /* 1 MB */
    gc->config.minHeapSize = 64 * 1024;       /* 64 KB */
    gc->config.heapGrowthFactor = 2;
    gc->config.enableCycleDetection = true;

    /* Statistics */
    memset(&gc->stats, 0, sizeof(PiGcStats));

    /* State flags */
    gc->isMarking = false;
    gc->isPaused = false;

    return;
}

/**
 * @brief   Free the garbage collection context resources.
 *
 * @param[in] env Environment containing the GC context.
 */
static void pi_FreeGcContext(PiEnv *const env)
{
    assert(env != NULL);

    /* Free all remaining GC objects */
    PiObject *obj = env->gc.objectList;

    while (obj)
    {
        PiObject *next = obj->gcNext;

        /* Free directly using heap (bypass handlers since we're shutting down) */
#if PI_PLATFORM_WINDOWS
        HeapFree((HANDLE)env->heap, 0, obj);
#else
        pi_PosixHeapFree((PiPosixHeap *)env->heap, obj);
#endif
        obj = next;
    }

    env->gc.objectList = NULL;

    /* Free global roots array */
    if (env->gc.globalRoots)
    {
#if PI_PLATFORM_WINDOWS
        HeapFree((HANDLE)env->heap, 0, env->gc.globalRoots);
#else
        pi_PosixHeapFree((PiPosixHeap *)env->heap, env->gc.globalRoots);
#endif
        env->gc.globalRoots = NULL;
    }

    env->gc.globalRootCount = 0;
    env->gc.globalRootCap = 0;

    return;
}

/**
 * @brief   Free child environments recursively.
 *
 * @param[in] env Environment whose children to free.
 */
static void pi_FreeChildEnvironments(PiEnv *const env)
{
    if (!env || !env->relation.children)
        return;

    for (size_t i = 0; i < env->relation.childCount; i++)
    {
        if (env->relation.children[i])
            piFreeEnv(env->relation.children[i]);
    }

    /* Free children array using standard free since it was allocated before heap handlers */
    free(env->relation.children);

    env->relation.children = NULL;
    env->relation.childCount = 0;
    env->relation.childCap = 0;

    return;
}

/* Forward declaration for pi_RemoveChildFromParent (defined after piCreateChildEnv) */
static void pi_RemoveChildFromParent(PiEnv *const parent, PiEnv *const child);

/* =---- Public API ------------------------------------------------= */

PI_Api(PiEnv *) piInitEnvWithSize(PiEnv *const env, size_t heapSize)
{
    assert(env != NULL);

    /* Clear entire structure */
    memset(env, 0, sizeof(PiEnv));

    /* Enforce minimum heap size */
    if (heapSize < PI_ENV_HEAP_MIN_SIZE)
        heapSize = PI_ENV_HEAP_MIN_SIZE;

    /* Create platform-specific heap */
#if PI_PLATFORM_WINDOWS
    {
        DWORD heapFlags = 0;

#ifdef PI_DEBUG
        heapFlags |= HEAP_GENERATE_EXCEPTIONS;
#endif

        env->heap = HeapCreate(heapFlags, heapSize, PI_ENV_HEAP_MAX_SIZE);

        if (!env->heap)
            piRaiseError("failed to create Windows heap", NULL);

        /* Set Windows handlers */
        env->mem.alloc = pi_WindowsAllocHandler;
        env->mem.realloc = pi_WindowsReallocHandler;
        env->mem.free = pi_WindowsFreeHandler;
    }
#else
    {
        env->heap = pi_PosixHeapCreate(heapSize);

        if (!env->heap)
            piRaiseError("failed to create POSIX heap", NULL);

        /* Set POSIX handlers */
        env->mem.alloc = pi_PosixAllocHandler;
        env->mem.realloc = pi_PosixReallocHandler;
        env->mem.free = pi_PosixFreeHandler;
    }
#endif

    /* Store heap size */
    env->heapSize = heapSize;

    /* Initialize GC context */
    pi_InitGcContext(&env->gc);

    /* Initialize state */
    env->state = PI_ENV_STATE_INIT;
    env->flags = PI_ENV_FLAG_OWNS_HEAP;
    env->exitCode = 0;

    /* Initialize relation */
    env->relation.parent = NULL;
    env->relation.children = NULL;
    env->relation.childCount = 0;
    env->relation.childCap = 0;
    env->relation.toParent = NULL;

    /* Initialize I/O handlers with platform defaults */
    env->io = piGetDefaultIoHandlers();

    /* Initialize standard streams (NULL for now, can be set later) */
    env->std.in = NULL;
    env->std.out = NULL;
    env->std.err = NULL;

    /* Initialize working directory */
    env->workingDir = NULL;
    env->workingDirSize = 0;

    /* Initialize capabilities with full permissions (root environment) */
    piCapabilityContextInitAll(&env->caps);

    /* Initialize error handling */
    env->errorJmp = NULL;

    /* Initialize user data */
    env->userData = NULL;

    return env;
}

PI_Api(PiEnv *) piInitEnv(PiEnv *const env)
{
    return piInitEnvWithSize(env, PI_ENV_HEAP_DEFAULT_SIZE);
}

PI_Api(PiEnv *) piFreeEnv(PiEnv *const env)
{
    if (!env)
        return NULL;

    if (env->state == PI_ENV_STATE_UNINIT)
        return env;

    /* Set state to terminated */
    env->state = PI_ENV_STATE_TERMINATED;

    /* Remove from parent's children list (if we are a child) */
    if (env->relation.parent)
    {
        pi_RemoveChildFromParent(env->relation.parent, env);
        env->relation.parent = NULL;
    }

    /* Free child environments first */
    pi_FreeChildEnvironments(env);
    /* Free GC resources (while heap is still valid) */
    pi_FreeGcContext(env);

    /* Destroy platform-specific heap (only if we own it) */
    if (env->flags & PI_ENV_FLAG_OWNS_HEAP)
    {
        if (env->heap)
#if PI_PLATFORM_WINDOWS
            HeapDestroy((HANDLE)env->heap);
#else
            pi_PosixHeapDestroy((PiPosixHeap *)env->heap);
#endif

        env->heap = NULL;
    }

    /* Clear memory handlers */
    env->mem.alloc = NULL;
    env->mem.realloc = NULL;
    env->mem.free = NULL;

    /* Clear I/O handlers */
    memset(&env->io, 0, sizeof(env->io));

    /* Clear standard streams */
    env->std.in = NULL;
    env->std.out = NULL;
    env->std.err = NULL;

    /* Note: workingDir was allocated from env heap, already freed */
    env->workingDir = NULL;
    env->workingDirSize = 0;

    /* Mark as uninitialized */
    env->state = PI_ENV_STATE_UNINIT;
    env->flags = PI_ENV_FLAG_NONE;

    return NULL;
}

/* =---- Error Handling --------------------------------------------= */

PI_NORET PI_Api(void) piEnvRaiseError(PiEnv *const env, const char *const format, ...)
{
    va_list args;

    va_start(args, format);
    piErrorAtV(__func__, __FILE__, __LINE__, format, args);
    va_end(args);

    /* If env has an error handler, longjmp to it */
    if (PI_LIKELY(env && env->errorJmp))
        longjmp(*env->errorJmp, 1);

    /* Otherwise, use global error handler */
    piRaiseError("an error occourred", NULL);
}

/* =---- Capability Enforcement ------------------------------------= */

PI_Api(bool) piEnvRequireCapability(PiEnv *const env, const PiCapability cap, const char *const what)
{
    if (!env)
    {
        piRaiseError("NULL environment", NULL);
        /* Not reached */
    }

    if (piHasCapability(&env->caps, cap))
        return true;

    /* Capability not granted - raise error */
    piEnvRaiseError(env, "permission denied (%s requires capability 0x%08X)", what ? what : "operation", (unsigned)cap);
    /* Not reached */
}

/* =---- Child Environment -----------------------------------------= */

/**
 * @brief   Add a child to parent's children array.
 *
 * @param[in] parent  Parent environment.
 * @param[in] child   Child to add.
 *
 * @return  true on success, false on failure.
 */
static bool pi_AddChildToParent(PiEnv *const parent, PiEnv *const child)
{
    /* Grow children array if needed */
    if (parent->relation.childCount >= parent->relation.childCap)
    {
        size_t newCap;
        PiEnv **newChildren;

        newCap = parent->relation.childCap == 0 ? 4 : parent->relation.childCap * 2;
        newChildren = (PiEnv **)realloc(parent->relation.children, newCap * sizeof(PiEnv *));

        if (!newChildren)
            return false;

        parent->relation.children = newChildren;
        parent->relation.childCap = newCap;
    }

    parent->relation.children[parent->relation.childCount++] = child;

    return true;
}

/**
 * @brief   Remove a child from parent's children array.
 *
 * @param[in] parent  Parent environment.
 * @param[in] child   Child to remove.
 */
static void pi_RemoveChildFromParent(PiEnv *const parent, PiEnv *const child)
{
    size_t i;

    if (!parent || !parent->relation.children)
        return;

    for (i = 0; i < parent->relation.childCount; i++)
    {
        if (parent->relation.children[i] == child)
        {
            /* Shift remaining elements */
            for (size_t j = i; j < parent->relation.childCount - 1; j++)
                parent->relation.children[j] = parent->relation.children[j + 1];

            parent->relation.childCount--;

            return;
        }
    }

    return;
}

PI_Api(PiEnv *) piCreateChildEnv(PiEnv *const parent, PiEnv *const child, const uint32_t caps, const size_t heapSize)
{
    if (!parent || !child)
        return NULL;

    /* Parent must have ENV_SPAWN capability */
    if (!piHasCapability(&parent->caps, PI_CAP_ENV_SPAWN))
    {
        piEnvRaiseError(parent, "cannot spawn child environment (PI_CAP_ENV_SPAWN not granted)");
        return NULL; /* Not reached */
    }

    /* Initialize child with its own heap */
    if (!piInitEnvWithSize(child, heapSize == 0 ? PI_ENV_HEAP_DEFAULT_SIZE : heapSize))
        return NULL;

    /* Derive capabilities from parent (intersection of requested and parent's caps) */
    piCapabilityContextDerive(&child->caps, &parent->caps, caps);

    /* Set up parent/child relationship */
    child->relation.parent = parent;
    child->flags |= PI_ENV_FLAG_IS_CHILD;

    /* Add child to parent's children list */
    if (!pi_AddChildToParent(parent, child))
    {
        piFreeEnv(child);
        return NULL;
    }

    /* Inherit I/O handlers from parent (child can override later) */
    child->io = parent->io;
    /* Inherit standard streams from parent (child can override later) */
    child->std = parent->std;

    return child;
}

/* =------------------------------------------------------------= */
