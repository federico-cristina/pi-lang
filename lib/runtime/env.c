#include "pi/runtime/env.h"

#ifdef _WIN32
#   include <Windows.h>
#endif

/* =---- Execution Environments --------------------------------= */

#ifdef _WIN32
static void *pi_DefaultAllocHandler(PiEnv *const env, const size_t size)
{
    assert(env != NULL);

    if (!env->isInit)
        piRaiseError("tried to allocate memory in uninitialized environment", NULL);

    const DWORD allocFlags =
          HEAP_ZERO_MEMORY
#   if PI_DEBUG
        | HEAP_GENERATE_EXCEPTIONS
#   endif
        ;

    /* Allocates the object on the environment heap */
    void *const result = (void *)HeapAlloc((HANDLE)env->heap, allocFlags, (SIZE_T)size);

    /* Checks if the allocation succeded */
    if (!result)
        piRaiseErrorAt(
            __func__,
            __FILE__,
            __LINE__,
            "cannot allocate memory in the heap"
        );

    return result;
}

static void *pi_DefaultReallocHandler(PiEnv *const env, void *const block, const size_t newSize)
{
    assert(env != NULL);

    if (!env->isInit)
        piRaiseError("tried to reallocate memory in uninitialized environment", NULL);

    const DWORD allocFlags =
          HEAP_ZERO_MEMORY
#   if PI_DEBUG
        | HEAP_GENERATE_EXCEPTIONS
#   endif
        ;

    /* Allocates the object on the environment heap */
    void *const result = (void *)HeapReAlloc((HANDLE)env->heap, allocFlags, (LPVOID)block, (SIZE_T)newSize);

    /* Checks if the allocation succeded */
    if (!result)
        piRaiseErrorAt(
            __func__,
            __FILE__,
            __LINE__,
            "cannot reallocate memory in the heap"
        );

    return result;
}

static void pi_DefaultFreeHandler(PiEnv *const env, void *const block)
{
    assert(env != NULL);

    if (!env->isInit)
        piRaiseError("tried to free memory in uninitialized environment", NULL);

    if (!HeapFree((HANDLE)env->heap, 0, (LPVOID)block))
        piRaiseError("cannot free object at <0x%p>", block);

    return;
}
#endif

/**
 * +---- Env ------------------------------+
 */

PiEnv *piInitEnv(PiEnv *const env)
{
    assert(env != NULL);

    /* Marks the environment as initialized */
    env->isInit = true;

#ifdef _WIN32
    const DWORD heapFlags =
          0
#   if PI_DEBUG
        | HEAP_CREATE_ENABLE_TRACING
        | HEAP_GENERATE_EXCEPTIONS
#   endif
        ;

    /* Creates the environment heap structure */
    env->heap = (void *)HeapCreate(heapFlags,
        PI_ENV_HEAP_SIZE_MIN,
        PI_ENV_HEAP_SIZE_MAX
    );

    /* Sets default memory allocation handlers */
    env->handlers.alloc = (PiMemAllocHandlerFn)pi_DefaultAllocHandler;
    env->handlers.realloc = (PiMemReallocHandlerFn)pi_DefaultReallocHandler;
    env->handlers.free = (PiMemFreeHandlerFn)pi_DefaultFreeHandler;
#else
    /* Sets default memory allocation handlers */
    env->handlers.alloc = (PiMemAllocHandlerFn)pi_malloc;
    env->handlers.realloc = (PiMemReallocHandlerFn)pi_realloc;
    env->handlers.free = (PiMemFreeHandlerFn)free;
#endif

    return env;
}

PiEnv *piFreeEnv(PiEnv *const env)
{
    assert(env != NULL);

    /* Marks the environment as freed */
    env->isInit = false;

#ifdef _WIN32
    /* Destroys the environment heap structure */
    if (!HeapDestroy(env->heap))
        piRaiseError("cannot destroy heap");
    else
        env->heap = NULL;
#endif

    /* Resets default memory allocation handlers */
    env->handlers.alloc = NULL;
    env->handlers.realloc = NULL;
    env->handlers.free = NULL;

    return env;
}

/* =------------------------------------------------------------= */
