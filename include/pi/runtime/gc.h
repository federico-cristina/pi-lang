#pragma once

/**
 * @file        gc.h
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
 * @brief       Hybrid garbage collection system (reference counting + mark-and-sweep).
 */

#ifndef _PI_RUNTIME_GC_H
#define _PI_RUNTIME_GC_H

#include "pi/common/common.h"

#include "pi/runtime/object.h"
#include "pi/runtime/value.h"

PI_C_HEADER_BEGIN

/* =---- Garbage Collection --------------------------------= */

/* Forward declarations */
typedef struct _pi_Environment PiEnv;

/**
 * +---- GcStats ---------------------------+
 */

/**
 * @brief   Garbage collection statistics.
 *
 * Tracks memory usage and collection performance metrics.
 */
typedef struct _pi_GcStats
{
    /**
     * @brief   Total number of allocated objects.
     */
    size_t objectCount;
    /**
     * @brief   Total bytes in managed objects.
     */
    size_t bytesAllocated;
    /**
     * @brief   Number of GC cycles run.
     */
    size_t collectionCount;
    /**
     * @brief   Objects freed since last cycle.
     */
    size_t objectsFreed;
    /**
     * @brief   Bytes freed since last cycle.
     */
    size_t bytesFreed;
} PiGcStats;

/**
 * +---- GcConfig --------------------------+
 */

/**
 * @brief   Garbage collection configuration.
 *
 * Controls GC behavior and thresholds.
 */
typedef struct _pi_GcConfig
{
    /**
     * @brief   Trigger GC when bytesAllocated exceeds this.
     */
    size_t nextGcThreshold;
    /**
     * @brief   Minimum heap size before first GC.
     */
    size_t minHeapSize;
    /**
     * @brief   Multiplier for nextGcThreshold (e.g., 2).
     */
    size_t heapGrowthFactor;
    /**
     * @brief   Enable mark-and-sweep cycle detection.
     */
    bool   enableCycleDetection;
} PiGcConfig;

/**
 * +---- GcContext -------------------------+
 */

/**
 * @brief   Garbage collection context.
 *
 * Contains all state needed for garbage collection, including object lists,
 * root sets, configuration, and statistics. This structure is embedded in
 * PiEnv to integrate GC with the runtime environment.
 */
typedef struct _pi_GcContext
{
    /* Object lists */
    PiObject       *objectList;         /**< All allocated objects (doubly-linked) */
    PiObject       *grayList;           /**< Gray objects for incremental marking (future) */

    /* Root sets */
    PiValueArray   *vmStack;            /**< Pointer to VM evaluation stack (root) */
    PiObject     ***globalRoots;        /**< Array of pointers to global root pointers */
    size_t          globalRootCount;    /**< Number of global roots */
    size_t          globalRootCap;      /**< Capacity of globalRoots array */

    /* Configuration and statistics */
    PiGcConfig      config;             /**< GC configuration */
    PiGcStats       stats;              /**< GC statistics */

    /* State flags */
    bool            isMarking;          /**< True during mark phase */
    bool            isPaused;           /**< True when GC is disabled */
} PiGcContext;

/**
 * +---- GC Allocation ---------------------+
 */

/**
 * @brief   Allocate a new GC-managed object.
 *
 * Allocates memory for an object of the specified type and size, initializes
 * the object header, and adds it to the GC's object list. The object starts
 * with a reference count of 1.
 *
 * If the allocation would exceed the GC threshold, a collection cycle is
 * triggered before allocation. If allocation fails, an emergency GC is
 * attempted before raising an error.
 *
 * @param[in,out] env  Environment context containing GC state.
 * @param[in]     type Object type (from PiObjectType enum).
 * @param[in]     size Total size in bytes (including header).
 *
 * @return Pointer to initialized object with refcount = 1.
 *
 * @note The returned object is owned by the caller. Use piGcRetain() to
 *       add additional references or piGcRelease() to release ownership.
 *
 * @note Triggers GC if bytesAllocated >= nextGcThreshold.
 *
 * @warning Raises fatal error if allocation fails after emergency GC.
 */
PI_Api(PiObject *) piGcAlloc(PiEnv *const env, const PiObjectType type, const size_t size);

/**
 * +---- GC Collection ---------------------+
 */

/**
 * @brief   Manually trigger garbage collection.
 *
 * Performs a complete mark-and-sweep collection cycle:
 * 1. CLEAR: Clears all mark bits on all objects
 * 2. MARK:  Marks objects reachable from roots (VM stack, global roots)
 * 3. SWEEP: Frees all unmarked objects
 *
 * After collection, adjusts the GC threshold based on current heap size
 * and the configured growth factor.
 *
 * @param[in,out] env Environment context containing GC state.
 *
 * @return Number of objects freed during this collection.
 *
 * @note Does nothing if GC is paused (isPaused = true).
 *
 * @note This is a stop-the-world collection. All VM execution should be
 *       paused during this call.
 */
PI_Api(size_t) piGcCollect(PiEnv *const env);

/**
 * +---- Reference Counting ----------------+
 */

/**
 * @brief   Increment object reference count.
 *
 * Adds a reference to the object, preventing it from being freed by
 * reference counting. If the refcount would overflow (>= 65535), the
 * object is marked as FROZEN to prevent premature collection; it will
 * only be freed by mark-and-sweep.
 *
 * @param[in,out] obj Object to retain (may be NULL).
 *
 * @return Same object pointer (for chaining).
 *
 * @note NULL-safe: returns NULL if obj is NULL.
 *
 * @note Frozen objects can only be freed by mark-and-sweep.
 *
 * @example
 *     PiObject *obj = piGcAlloc(env, PI_OBJECT_TYPE_STRING, sizeof(PiStringObject) + 10);
 *     piGcRetain(obj);  // refcount: 1 -> 2
 */
PI_Api(PiObject *) piGcRetain(PiObject *const obj);
/**
 * @brief   Decrement object reference count.
 *
 * Removes a reference from the object. When the refcount reaches 0,
 * the object is immediately freed (unless it's FROZEN).
 *
 * FROZEN objects (refcount overflowed) cannot be freed by this function;
 * they require mark-and-sweep collection.
 *
 * @param[in,out] env Environment context containing GC state.
 * @param[in,out] obj Object to release (may be NULL).
 *
 * @note NULL-safe: does nothing if obj is NULL.
 *
 * @note Immediate collection when refcount reaches 0 (for non-frozen objects).
 *
 * @warning Do not access obj after calling this function; it may have been freed.
 *
 * @example
 *     piGcRelease(env, obj);  // refcount: 2 -> 1
 *     piGcRelease(env, obj);  // refcount: 1 -> 0, object freed
 */
PI_Api(void) piGcRelease(PiEnv *const env, PiObject *const obj);

/**
 * +---- Root Management -------------------+
 */

/**
 * @brief   Register a global root pointer.
 *
 * Adds a pointer to the global roots set. Objects referenced by global
 * roots are considered reachable and will not be collected during
 * mark-and-sweep.
 *
 * This is used for long-lived objects that should persist across GC
 * cycles, such as global variables, module objects, or cached values.
 *
 * @param[in,out] env  Environment context.
 * @param[in]     root Pointer to PiObject* that should remain rooted.
 *
 * @note The root parameter is a pointer-to-pointer because the GC needs
 *       to follow the indirection to find the actual object.
 *
 * @note It's safe to register the same root multiple times, but each
 *       registration should be balanced with a corresponding removal.
 *
 * @example
 *     PiObject *globalVar = piGcAlloc(env, ...);
 *     piGcAddGlobalRoot(env, &globalVar);
 */
PI_Api(void) piGcAddGlobalRoot(PiEnv *const env, PiObject **const root);
/**
 * @brief   Unregister a global root pointer.
 *
 * Removes a pointer from the global roots set. The object will no longer
 * be protected from garbage collection (unless referenced elsewhere).
 *
 * @param[in,out] env  Environment context.
 * @param[in]     root Pointer to remove from roots.
 *
 * @note Does nothing if the root was not previously registered.
 *
 * @example
 *     piGcRemoveGlobalRoot(env, &globalVar);
 *     globalVar = NULL;  // Good practice: clear pointer after unrooting
 */
PI_Api(void) piGcRemoveGlobalRoot(PiEnv *const env, PiObject **const root);

/**
 * @brief   Set VM stack as root set.
 *
 * Registers the VM evaluation stack as a root for garbage collection.
 * All objects referenced by values on the stack are considered reachable.
 *
 * This should be called by the VM when starting execution and cleared
 * when execution completes.
 *
 * @param[in,out] env   Environment context.
 * @param[in]     stack Pointer to VM evaluation stack (or NULL to clear).
 *
 * @note Only one VM stack can be registered at a time. Setting a new
 *       stack replaces the previous one.
 *
 * @example
 *     PiValueArray evalStack;
 *     piInitValueArray(&evalStack, 256);
 *     piGcSetVmStack(env, &evalStack);
 *     // ... VM execution ...
 *     piGcSetVmStack(env, NULL);
 *     piFreeValueArray(&evalStack);
 */
PI_Api(void) piGcSetVmStack(PiEnv *const env, PiValueArray *const stack);

/**
 * +---- GC Utilities ----------------------+
 */

/**
 * @brief   Temporarily pause or resume garbage collection.
 *
 * When paused, piGcCollect() becomes a no-op and automatic collections
 * are disabled. This is useful for critical sections where GC would be
 * unsafe or undesirable.
 *
 * @param[in,out] env    Environment context.
 * @param[in]     paused True to pause GC, false to resume.
 *
 * @note GC pause/resume calls can be nested; keep a counter if needed.
 *
 * @warning Long pauses can lead to memory exhaustion. Keep pauses short.
 *
 * @example
 *     piGcSetPaused(env, true);
 *     // ... critical section ...
 *     piGcSetPaused(env, false);
 */
PI_Api(void) piGcSetPaused(PiEnv *const env, const bool paused);

/**
 * @brief   Get garbage collection statistics.
 *
 * Returns a copy of the current GC statistics, including object count,
 * bytes allocated, collection count, and objects/bytes freed.
 *
 * @param[in] env Environment context.
 *
 * @return Copy of current GC statistics.
 *
 * @note This is a snapshot; values may change immediately after return.
 *
 * @example
 *     PiGcStats stats = piGcGetStats(env);
 *     printf("Objects: %zu, Bytes: %zu\n", stats.objectCount, stats.bytesAllocated);
 */
PI_Api(PiGcStats) piGcGetStats(const PiEnv *const env);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
