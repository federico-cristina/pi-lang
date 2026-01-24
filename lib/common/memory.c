#include "pi/common/memory.h"
#include "pi/common/error.h"

#ifdef _WIN32
#   include <malloc.h>  /* For _aligned_malloc/_aligned_free */
#endif

/* =---- Memory Tracking (Optional) ----------------------------= */

#ifdef PI_ENABLE_MEMORY_TRACKING
#   include <stdio.h>  /* For fprintf in memory tracking */
/**
 * @brief   Memory tracking statistics.
 */
typedef struct _pi_MemoryStats
{
    size_t totalAllocated;
    size_t peakAllocated;
    size_t allocationCount;
} PiMemoryStats;

/**
 * @brief   Global memory stats.
 */
static PI_THREAD_LOCAL PiMemoryStats g_MemoryStats = {0};

/**
 * @brief   Records an allocation.
 */
static inline void pi_TrackAllocation(size_t size)
{
    g_MemoryStats.totalAllocated += size;
    g_MemoryStats.allocationCount++;

    if (g_MemoryStats.totalAllocated > g_MemoryStats.peakAllocated)
        g_MemoryStats.peakAllocated = g_MemoryStats.totalAllocated;

    return;
}

/**
 * @brief   Records a deallocation.
 */
static inline void pi_TrackDeallocation(size_t size)
{
    g_MemoryStats.totalAllocated -= size;

    return;
}

PI_Api(size_t) piGetAllocatedMemory(void)
{
    return g_MemoryStats.totalAllocated;
}

PI_Api(size_t) piGetPeakMemory(void)
{
    return g_MemoryStats.peakAllocated;
}

PI_Api(size_t) piGetAllocationCount(void)
{
    return g_MemoryStats.allocationCount;
}

PI_Api(void) piPrintMemoryStats(void)
{
    fprintf(stderr, PI_TITLE_INFO ": Memory Statistics\n");
    fprintf(stderr, "  Current: %zu bytes\n", g_MemoryStats.totalAllocated);
    fprintf(stderr, "  Peak:    %zu bytes\n", g_MemoryStats.peakAllocated);
    fprintf(stderr, "  Allocs:  %zu\n", g_MemoryStats.allocationCount);

    return;
}
#endif

/* =---- Memory Allocation Helpers -----------------------------= */

#ifndef PI_XALLOC_ERROR_MESSAGE
#   define PI_XALLOC_ERROR_MESSAGE "cannot allocate enough memory"
#endif

#ifndef PI_REALLOC_ERROR_MESSAGE
#   define PI_REALLOC_ERROR_MESSAGE "cannot reallocate memory"
#endif

/**
 * @brief   Checks if allocation succeeded.
 */
static inline const void *pi_CheckNotNull(const void *const block, const char *const func, const char *const file, const int line, const char *const message)
{
    if (PI_UNLIKELY(!block))
        piFatalErrorAt(func, file, line, "%s", message ? message : "memory allocation failed");

    return block;
}

/* =---- Memory Allocation Functions ---------------------------= */

PI_Api(void *) piMalloc(const size_t size)
{
    void *ptr = malloc(size);

#ifdef PI_ENABLE_MEMORY_TRACKING
    if (ptr)
        pi_TrackAllocation(size);
#endif

    return (void *)pi_CheckNotNull(ptr, __func__, __FILE__, __LINE__, PI_XALLOC_ERROR_MESSAGE);
}

PI_Api(void *) piCalloc(const size_t count, const size_t size)
{
    void *ptr = calloc(count, size);

#ifdef PI_ENABLE_MEMORY_TRACKING
    if (ptr)
        pi_TrackAllocation(count * size);
#endif

    return (void *)pi_CheckNotNull(ptr, __func__, __FILE__, __LINE__, PI_XALLOC_ERROR_MESSAGE);
}

PI_Api(void *) piRealloc(void *const block, const size_t newSize)
{
    return (void *)pi_CheckNotNull(realloc(block, newSize), __func__, __FILE__, __LINE__, PI_REALLOC_ERROR_MESSAGE);
}

PI_Api(void *) piResize(void *const block, const size_t oldSize, const size_t newSize)
{
    /* Free if newSize is 0 */
    if (newSize == 0)
    {
        free(block);
        return NULL;
    }

    /* Allocate or reallocate based on oldSize */
    return (oldSize > 0) ? piRealloc(block, newSize) : piMalloc(newSize);
}

PI_Api(void) piFree(void *ptr)
{
    if (ptr)
        free(ptr);

    return;
}

/* =---- Aligned Memory Allocation -----------------------------= */

PI_Api(void *) piMallocAligned(const size_t size, const size_t alignment)
{
    void *ptr;

    assert(size > 0 && alignment > 0);
    assert((alignment & (alignment - 1)) == 0); /* Must be power of 2 */

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L) && !defined(_WIN32)
    /* C11 standard aligned_alloc (not available on Windows) */
    ptr = aligned_alloc(alignment, size);
#elif defined(_WIN32)
    /* Windows-specific aligned malloc */
    ptr = _aligned_malloc(size, alignment);
#else
    /* POSIX fallback */
    if (posix_memalign(&ptr, alignment, size) != 0)
        ptr = NULL;
#endif

    return (void *)pi_CheckNotNull(ptr, __func__, __FILE__, __LINE__, PI_XALLOC_ERROR_MESSAGE);
}

PI_Api(void) piFreeAligned(void *ptr)
{
    if (!ptr)
        return;

#ifdef _WIN32
    _aligned_free(ptr);
#else
    free(ptr);
#endif

    return;
}

/* =------------------------------------------------------------= */
