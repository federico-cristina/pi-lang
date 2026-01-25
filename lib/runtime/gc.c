#include "pi/runtime/env.h"
#include "pi/runtime/gc.h"

#include <string.h>

/* =---- Internal Functions ------------------------------------= */

/**
 * @brief   Free an object immediately (internal).
 *
 * Removes the object from the GC list, calls type-specific finalizers, updates statistics,
 * and frees memory.
 *
 * @param[in,out] env Environment context.
 * @param[in,out] obj Object to free.
 */
static void pi_GcFreeObject(PiEnv *const env, PiObject *const obj)
{
    assert(env != NULL);
    assert(obj != NULL);

    /* Remove from GC object list (doubly-linked) */
    if (obj->gcNext)
        obj->gcNext->gcPrev = obj->gcPrev;

    if (obj->gcPrev)
        obj->gcPrev->gcNext = obj->gcNext;
    else
        env->gc.objectList = obj->gcNext; /* Was head */

    /* Call type-specific finalizer (if needed) */
    switch (obj->type)
    {
    case PI_OBJECT_TYPE_STRING:
        /* Strings have no internal references - no cleanup needed */
        break;

    /* Future types: release internal references before freeing */
    case PI_OBJECT_TYPE_ARRAY:
    case PI_OBJECT_TYPE_NATIVE:
        /* TODO: Implement finalizers for future object types */
        break;

    default:
        piUnreachable();
        break;
    }

    /* Update statistics */
    env->gc.stats.objectCount--;
    env->gc.stats.bytesAllocated -= obj->size;
    env->gc.stats.objectsFreed++;
    env->gc.stats.bytesFreed += obj->size;

    /* Free memory using environment handler */
    env->mem.free(env, obj);

    return;
}

/**
 * @brief   Clear all mark bits (Phase 1 of mark-and-sweep).
 *
 * Iterates through all objects and clears MARKED and GRAY flags.
 *
 * @param[in,out] env Environment context.
 */
static void pi_GcClearMarks(PiEnv *const env)
{
    assert(env != NULL);

    PiObject *obj = env->gc.objectList;

    while (obj)
    {
        obj->flags &= ~(PI_OBJECT_FLAG_MARKED | PI_OBJECT_FLAG_GRAY);
        obj = obj->gcNext;
    }

    return;
}

/**
 * @brief   Recursively mark an object and its references (Phase 2 helper).
 *
 * Marks the object as MARKED and recursively marks any objects it references.
 * This implements depth-first traversal for mark-and-sweep.
 *
 * @param[in,out] obj Object to mark.
 */
static void pi_GcMarkObject(PiObject *const obj)
{
    if (!obj || (obj->flags & PI_OBJECT_FLAG_MARKED))
        return; /* Already marked or NULL */

    /* Mark this object */
    obj->flags |= PI_OBJECT_FLAG_MARKED;

    /* Mark internal references based on type */
    switch (obj->type)
    {
    case PI_OBJECT_TYPE_STRING:
        /* Strings have no internal object references */
        break;

    case PI_OBJECT_TYPE_ARRAY:
        /* TODO: Mark array elements when arrays are implemented */
        /* PiArrayObject *arr = (PiArrayObject *)obj;
         * for (uint32_t i = 0; i < arr->count; i++) {
         *     if (piIsObject(arr->data[i]))
         *         pi_GcMarkObject(piAsObject(arr->data[i]));
         * }
         */
        break;

    case PI_OBJECT_TYPE_NATIVE:
        /* Native functions have no internal references */
        break;

    default:
        break;
    }

    return;
}

/**
 * @brief   Mark objects reachable from roots (Phase 2 of mark-and-sweep).
 *
 * Marks all objects referenced by:
 * - VM evaluation stack (if registered)
 * - Global root pointers
 *
 * @param[in,out] env Environment context.
 */
static void pi_GcMarkRoots(PiEnv *const env)
{
    assert(env != NULL);

    /* Mark VM stack roots */
    if (env->gc.vmStack)
    {
        for (uint32_t i = 0; i < env->gc.vmStack->count; i++)
        {
            PiValue val = env->gc.vmStack->data[i];

            if (piIsObject(val))
                pi_GcMarkObject(piAsObject(val));
        }
    }

    /* Mark global roots */
    for (size_t i = 0; i < env->gc.globalRootCount; i++)
    {
        PiObject **const root = env->gc.globalRoots[i];

        if (root && *root)
            pi_GcMarkObject(*root);
    }

    return;
}

/**
 * @brief   Sweep unmarked objects (Phase 3 of mark-and-sweep).
 *
 * Iterates through all objects and frees those that are not marked.
 * Marked objects survive this collection cycle.
 *
 * @param[in,out] env Environment context.
 */
static void pi_GcSweep(PiEnv *const env)
{
    assert(env != NULL);

    PiObject *obj = env->gc.objectList;

    while (obj)
    {
        PiObject *const next = obj->gcNext;

        if (!(obj->flags & PI_OBJECT_FLAG_MARKED))
        {
            /* Unreachable object - free it */
            pi_GcFreeObject(env, obj);
        }

        obj = next;
    }

    return;
}

/* =---- Garbage Collection --------------------------------= */

/**
 * +---- GC Allocation ---------------------+
 */

PI_Api(PiObject *) piGcAlloc(PiEnv *const env, const PiObjectType type, const size_t size)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));
    assert(size >= sizeof(PiObject));

    /* Check if GC should run BEFORE allocation */
    if (env->gc.stats.bytesAllocated >= env->gc.config.nextGcThreshold)
        piGcCollect(env);

    /* Allocate using environment handler */
    PiObject *obj = (PiObject *)env->mem.alloc(env, size);

    if (PI_UNLIKELY(!obj))
    {
        /* Emergency GC and retry */
        piGcCollect(env);

        obj = (PiObject *)env->mem.alloc(env, size);

        if (!obj)
            piRaiseError("out of memory (cannot allocate %zu bytes)", (void *)size);
    }

    /* Initialize object header */
    obj->gcNext = env->gc.objectList;
    obj->gcPrev = NULL;
    obj->type = type;
    obj->flags = PI_OBJECT_FLAG_NONE;
    obj->refCount = 1; /* Start with refcount of 1 */
    obj->size = (uint32_t)size;
    obj->padding = 0;

    /* Add to object list */
    if (env->gc.objectList)
        env->gc.objectList->gcPrev = obj;

    env->gc.objectList = obj;

    /* Update statistics */
    env->gc.stats.objectCount++;
    env->gc.stats.bytesAllocated += size;

    return obj;
}

/**
 * +---- GC Collection ---------------------+
 */

PI_Api(size_t) piGcCollect(PiEnv *const env)
{
    assert(env != NULL);
    assert(piEnvIsInit(env));

    if (env->gc.isPaused)
        return 0; /* GC disabled */

    const size_t objectsBefore = env->gc.stats.objectCount;

    /* Phase 1: CLEAR - Clear all mark bits */
    pi_GcClearMarks(env);
    /* Phase 2: MARK - Mark from roots */
    pi_GcMarkRoots(env);
    /* Phase 3: SWEEP - Free unmarked objects */
    pi_GcSweep(env);

    /* Update statistics */
    env->gc.stats.collectionCount++;

    /* Adjust next threshold (heap growth factor) */
    env->gc.config.nextGcThreshold =
        env->gc.stats.bytesAllocated * env->gc.config.heapGrowthFactor;

    return objectsBefore - env->gc.stats.objectCount;
}

/**
 * +---- Reference Counting ----------------+
 */

PI_Api(PiObject *) piGcRetain(PiObject *const obj)
{
    if (!obj)
        return NULL;

    /* Check for refcount overflow */
    if (PI_UNLIKELY(obj->refCount >= UINT16_MAX - 1))
    {
        /* Mark as "sticky" - requires mark-and-sweep to free */
        obj->flags |= PI_OBJECT_FLAG_FROZEN;

        return obj;
    }

    obj->refCount++;

    return obj;
}

PI_Api(void) piGcRelease(PiEnv *const env, PiObject *const obj)
{
    if (!obj)
        return;

    /* Frozen objects cannot be freed by refcount */
    if (obj->flags & PI_OBJECT_FLAG_FROZEN)
        return;

    assert(obj->refCount > 0);

    obj->refCount--;

    /* Immediate collection when refcount reaches 0 */
    if (obj->refCount == 0)
        pi_GcFreeObject(env, obj);

    return;
}

/**
 * +---- Root Management -------------------+
 */

PI_Api(void) piGcAddGlobalRoot(PiEnv *const env, PiObject **const root)
{
    assert(env != NULL);
    assert(root != NULL);

    /* Grow array if needed */
    if (env->gc.globalRootCount >= env->gc.globalRootCap)
    {
        const size_t newCap = (env->gc.globalRootCap == 0) ? 8 : (env->gc.globalRootCap * 2);

        PiObject ***const newRoots =
            (PiObject ***)piRealloc(env->gc.globalRoots, newCap * sizeof(PiObject **));

        if (!newRoots)
            piRaiseError("out of memory: cannot grow global roots array", NULL);

        env->gc.globalRoots = newRoots;
        env->gc.globalRootCap = newCap;
    }

    /* Add root */
    env->gc.globalRoots[env->gc.globalRootCount++] = root;

    return;
}

PI_Api(void) piGcRemoveGlobalRoot(PiEnv *const env, PiObject **const root)
{
    assert(env != NULL);
    assert(root != NULL);

    /* Find and remove root (linear search) */
    for (size_t i = 0; i < env->gc.globalRootCount; i++)
    {
        if (env->gc.globalRoots[i] == root)
        {
            /* Swap with last element and decrement count */
            env->gc.globalRoots[i] = env->gc.globalRoots[env->gc.globalRootCount - 1];
            env->gc.globalRootCount--;

            return;
        }
    }

    /* Root not found - this is okay, no-op */
    return;
}

PI_Api(void) piGcSetVmStack(PiEnv *const env, PiValueArray *const stack)
{
    assert(env != NULL);

    env->gc.vmStack = stack;

    return;
}

/**
 * +---- GC Utilities ----------------------+
 */

PI_Api(void) piGcSetPaused(PiEnv *const env, const bool paused)
{
    assert(env != NULL);

    env->gc.isPaused = paused;

    return;
}

PI_Api(PiGcStats) piGcGetStats(const PiEnv *const env)
{
    assert(env != NULL);

    return env->gc.stats;
}

/* =------------------------------------------------------------= */
