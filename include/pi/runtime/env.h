#pragma once

/**
 * @file        env.h
 *
 * @author      Federico Cristina <federico.cristina@outlook.it>
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
 * @brief       Runtime execution environment with isolated heap and custom handlers.
 *
 *              The PiEnv structure represents an isolated execution context with:
 *               - Its own heap for memory allocation
 *               - Its own garbage collector
 *               - Customizable memory and I/O handlers
 *               - Capability-based security model
 *               - Support for parent/child relationships
 */

#ifndef _PI_RUNTIME_ENV_H
#define _PI_RUNTIME_ENV_H

#include "pi/common/common.h"

#include "pi/runtime/object.h"
#include "pi/runtime/value.h"
#include "pi/runtime/gc.h"
#include "pi/runtime/io.h"
#include "pi/runtime/capability.h"

#include <setjmp.h>

PI_C_HEADER_BEGIN

/* =---- Configuration -----------------------------------------= */

#ifndef PI_ENV_HEAP_DEFAULT_SIZE
/**
 * @brief   Default heap size in bytes.
 */
#   define PI_ENV_HEAP_DEFAULT_SIZE 0
#endif

#ifndef PI_ENV_HEAP_MIN_SIZE
/**
 * @brief   Minimum heap size in bytes (0 = one page).
 */
#   define PI_ENV_HEAP_MIN_SIZE 0
#endif

#ifndef PI_ENV_HEAP_MAX_SIZE
/**
 * @brief   Maximum heap size in bytes (0 = unlimited).
 */
#   define PI_ENV_HEAP_MAX_SIZE 0
#endif

#ifndef PI_ENV_HEAP_ALIGNMENT
/**
 * @brief   Heap memory alignment in bytes.
 */
#   define PI_ENV_HEAP_ALIGNMENT 16
#endif

/* =---- Environment State -------------------------------------= */

/**
 * @brief   Environment lifecycle states.
 *
 * Environments transition through these states during their lifecycle:
 * - UNINIT -> INIT -> RUNNING -> TERMINATED
 * - RUNNING can also transition to SUSPENDED and back
 */
typedef enum _pi_EnvState
{
    /**
     * @brief   Environment is not initialized.
     */
    PI_ENV_STATE_UNINIT     = 0,
    /**
     * @brief   Environment is initialized but not running.
     */
    PI_ENV_STATE_INIT       = 1,
    /**
     * @brief   Environment is actively executing code.
     */
    PI_ENV_STATE_RUNNING    = 2,
    /**
     * @brief   Environment is temporarily suspended.
     */
    PI_ENV_STATE_SUSPENDED  = 3,
    /**
     * @brief   Environment has terminated (cleanup pending).
     */
    PI_ENV_STATE_TERMINATED = 4,
} PiEnvState;

/* =---- Memory Handlers ---------------------------------------= */

/**
 * @brief   Forward declaration of environment structure.
 */
typedef struct _pi_Environment PiEnv;

/**
 * @brief   Memory allocation handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] size  Number of bytes to allocate.
 *
 * @return  Pointer to allocated memory, or NULL on failure.
 */
typedef void *(*PiMemAllocFn)(PiEnv *const env, const size_t size);
/**
 * @brief   Memory reallocation handler function type.
 *
 * @param[in] env      Environment context.
 * @param[in] block    Pointer to existing block (or NULL).
 * @param[in] newSize  New size in bytes.
 *
 * @return  Pointer to reallocated memory, or NULL on failure.
 */
typedef void *(*PiMemReallocFn)(PiEnv *const env, void *const block, const size_t newSize);
/**
 * @brief   Memory deallocation handler function type.
 *
 * @param[in] env    Environment context.
 * @param[in] block  Pointer to memory block to free.
 */
typedef void (*PiMemFreeFn)(PiEnv *const env, void *const block);

/**
 * @brief   Memory handler function table.
 *
 * Contains function pointers for memory operations. These can be customized
 * to use different allocators (arena, pool, tracking allocators, etc.).
 */
typedef struct _pi_MemHandlers
{
    /**
     * @brief   Allocation function.
     */
    PiMemAllocFn    alloc;
    /**
     * @brief   Reallocation function.
     */
    PiMemReallocFn  realloc;
    /**
     * @brief   Deallocation function.
     */
    PiMemFreeFn     free;
} PiMemHandlers;

/* =---- Environment Flags -------------------------------------= */

/**
 * @brief   Environment configuration and state flags.
 */
typedef enum _pi_EnvFlags
{
    /**
     * @brief   No flags set.
     */
    PI_ENV_FLAG_NONE        = 0,
    /**
     * @brief   Environment is running in REPL mode.
     */
    PI_ENV_FLAG_REPL        = PI_BitFlag(1),
    /**
     * @brief   Environment received an interrupt signal.
     */
    PI_ENV_FLAG_INTERRUPT   = PI_BitFlag(2),
    /**
     * @brief   Environment owns its heap (should free on cleanup).
     */
    PI_ENV_FLAG_OWNS_HEAP   = PI_BitFlag(3),
    /**
     * @brief   Environment is a child of another environment.
     */
    PI_ENV_FLAG_IS_CHILD    = PI_BitFlag(4),
} PiEnvFlags;

/* =---- Environment Relation ----------------------------------= */

/**
 * @brief   Forward declaration for channel type.
 */
typedef struct _pi_Channel PiChannel;

/**
 * @brief   Environment parent/child relationship data.
 *
 * Tracks the hierarchy of environments for resource management
 * and inter-environment communication.
 */
typedef struct _pi_EnvRelation
{
    /**
     * @brief   Parent environment (NULL for root environments).
     */
    PiEnv          *parent;
    /**
     * @brief   Array of child environments.
     */
    PiEnv         **children;
    /**
     * @brief   Number of active child environments.
     */
    size_t          childCount;
    /**
     * @brief   Capacity of children array.
     */
    size_t          childCap;
    /**
     * @brief   Channel for communicating with parent.
     */
    PiChannel      *toParent;
} PiEnvRelation;

/* =---- Environment Structure ---------------------------------= */

/**
 * @brief   Runtime execution environment.
 *
 * Represents an isolated execution context with its own memory heap,
 * garbage collector, and customizable handlers. Environments can form
 * parent/child hierarchies and communicate through channels.
 *
 * @note    Use piInitEnv() to initialize and piFreeEnv() to cleanup.
 */
struct _pi_Environment
{
    /* ---- State (8 bytes) ---- */

    /**
     * @brief   Current environment state.
     */
    PiEnvState              state;
    /**
     * @brief   Environment flags bitfield.
     */
    uint32_t                flags;

    /* ---- Exit Status (8 bytes) ---- */

    /**
     * @brief   Exit code to return when environment terminates.
     */
    volatile int            exitCode;
    /**
     * @brief   Reserved for alignment.
     */
    uint32_t                _reserved0;

    /* ---- Heap (16 bytes) ---- */

    /**
     * @brief   Platform-specific heap handle.
     *
     * On Windows: HANDLE from HeapCreate()
     * On POSIX: Pointer to PiPosixHeap structure
     */
    void                   *heap;
    /**
     * @brief   Current heap size in bytes (for tracking).
     */
    size_t                  heapSize;

    /* ---- Memory Handlers (24 bytes) ---- */

    /**
     * @brief   Memory allocation handlers.
     */
    PiMemHandlers           mem;

    /* ---- I/O Handlers ---- */

    /**
     * @brief   I/O operation handlers.
     *
     * Contains function pointers for file, filesystem, and console I/O.
     * Can be customized to implement sandboxing or redirection.
     */
    PiIoHandlers            io;
    /**
     * @brief   Standard streams (stdin, stdout, stderr).
     *
     * Initialized during environment creation. Can be redirected to
     * files or other streams for I/O redirection.
     */
    PiStdStreams            std;

    /* ---- Working Directory ---- */

    /**
     * @brief   Environment-local working directory path.
     *
     * Each environment can have its own working directory, independent
     * of the process-wide current directory. NULL if not set.
     */
    char                   *workingDir;
    /**
     * @brief   Size of working directory buffer.
     */
    size_t                  workingDirSize;

    /* ---- Capabilities ---- */

    /**
     * @brief   Capability-based security context.
     *
     * Controls what operations this environment is allowed to perform.
     * Child environments inherit a subset of parent capabilities.
     */
    PiCapabilityContext     caps;

    /* ---- Garbage Collector ---- */

    /**
     * @brief   Garbage collection context.
     */
    PiGcContext             gc;

    /* ---- Hierarchy ---- */

    /**
     * @brief   Parent/child relationship data.
     */
    PiEnvRelation           relation;

    /* ---- Error Handling ---- */

    /**
     * @brief   Error recovery jump buffer.
     *
     * When set, errors raised via piEnvRaiseError() will longjmp to this
     * buffer instead of terminating the program. NULL if not set.
     */
    jmp_buf                *errorJmp;

    /* ---- User Data ---- */

    /**
     * @brief   User-defined data pointer.
     *
     * Can be used to attach application-specific context to the environment.
     */
    void                   *userData;
};

/* =---- Environment API ---------------------------------------= */

/**
 * @brief   Initialize a runtime environment.
 *
 * Sets up the heap, memory handlers, and GC context. After initialization,
 * the environment is in PI_ENV_STATE_INIT state.
 *
 * @param[out] env  Environment structure to initialize.
 *
 * @return  Pointer to initialized environment (same as input).
 *
 * @note    Call piFreeEnv() when done to release resources.
 */
PI_Api(PiEnv *) piInitEnv(PiEnv *const env);
/**
 * @brief   Initialize a runtime environment with custom heap size.
 *
 * @param[out] env       Environment structure to initialize.
 * @param[in]  heapSize  Initial heap size in bytes.
 *
 * @return  Pointer to initialized environment (same as input).
 */
PI_Api(PiEnv *) piInitEnvWithSize(PiEnv *const env, const size_t heapSize);

/**
 * @brief   Free a runtime environment.
 *
 * Releases all resources including heap, GC objects, and child environments.
 * After this call, the environment is in PI_ENV_STATE_UNINIT state.
 *
 * @param[in,out] env  Environment to free.
 *
 * @return  NULL (for convenience in assignments like `env = piFreeEnv(env)`).
 */
PI_Api(PiEnv *) piFreeEnv(PiEnv *const env);

/* =---- Environment State API ---------------------------------= */

/**
 * @brief   Get the current state of an environment.
 *
 * @param[in] env  Environment to query.
 *
 * @return  Current environment state.
 */
PI_InlineApi(PiEnvState) piEnvGetState(const PiEnv *const env)
{
    return env ? env->state : PI_ENV_STATE_UNINIT;
}

/**
 * @brief   Check if an environment is initialized.
 *
 * @param[in] env  Environment to check.
 *
 * @return  true if initialized, false otherwise.
 */
PI_InlineApi(bool) piEnvIsInit(const PiEnv *const env)
{
    return env && (env->state >= PI_ENV_STATE_INIT);
}
/**
 * @brief   Check if an environment is running.
 *
 * @param[in] env  Environment to check.
 *
 * @return  true if running, false otherwise.
 */
PI_InlineApi(bool) piEnvIsRunning(const PiEnv *const env)
{
    return env && (env->state == PI_ENV_STATE_RUNNING);
}

/**
 * @brief   Check if an environment has a specific flag set.
 *
 * @param[in] env   Environment to check.
 * @param[in] flag  Flag to test.
 *
 * @return  true if flag is set, false otherwise.
 */
PI_InlineApi(bool) piEnvHasFlag(const PiEnv *const env, PiEnvFlags flag)
{
    return env && ((env->flags & flag) != 0);
}
/**
 * @brief   Set a flag on an environment.
 *
 * @param[in,out] env   Environment to modify.
 * @param[in]     flag  Flag to set.
 */
PI_InlineApi(void) piEnvSetFlag(PiEnv *const env, const PiEnvFlags flag)
{
    if (env)
        env->flags |= flag;

    return;
}
/**
 * @brief   Clear a flag on an environment.
 *
 * @param[in,out] env   Environment to modify.
 * @param[in]     flag  Flag to clear.
 */
PI_InlineApi(void) piEnvClearFlag(PiEnv *const env, const PiEnvFlags flag)
{
    if (env)
        env->flags &= ~flag;

    return;
}

/* =---- Environment Memory API --------------------------------= */

/**
 * @brief   Allocate memory from environment's heap.
 *
 * @param[in] env   Environment context.
 * @param[in] size  Number of bytes to allocate.
 *
 * @return  Pointer to allocated memory, or NULL on failure.
 */
PI_InlineApi(void *) piEnvAlloc(PiEnv *const env, const size_t size)
{
    return (env && env->mem.alloc) ? env->mem.alloc(env, size) : NULL;
}
/**
 * @brief   Reallocate memory from environment's heap.
 *
 * @param[in] env      Environment context.
 * @param[in] block    Existing memory block (or NULL).
 * @param[in] newSize  New size in bytes.
 *
 * @return  Pointer to reallocated memory, or NULL on failure.
 */
PI_InlineApi(void *) piEnvRealloc(PiEnv *const env, void *const block, const size_t newSize)
{
    return (env && env->mem.realloc) ? env->mem.realloc(env, block, newSize) : NULL;
}
/**
 * @brief   Free memory back to environment's heap.
 *
 * @param[in] env    Environment context.
 * @param[in] block  Memory block to free.
 */
PI_InlineApi(void) piEnvFree(PiEnv *const env, void *const block)
{
    if (env && env->mem.free)
        env->mem.free(env, block);

    return;
}

/* =---- Environment Error API ---------------------------------= */

/**
 * @brief   Set the error handler for an environment.
 *
 * When set, errors raised via piEnvRaiseError() will longjmp to this
 * buffer instead of terminating the program.
 *
 * @param[in,out] env  Environment to configure.
 * @param[in]     jmp  Jump buffer for error recovery (NULL to disable).
 *
 * @example
 * @code
 *     jmp_buf errorJmp;
 *     piEnvSetErrorHandler(env, &errorJmp);
 *     if (setjmp(errorJmp) != 0) {
 *         // Error occurred, handle it
 *         return;
 *     }
 *     // ... code that might raise errors ...
 * @endcode
 */
PI_InlineApi(void) piEnvSetErrorHandler(PiEnv *const env, jmp_buf *const jmp)
{
    if (env)
        env->errorJmp = jmp;

    return;
}
/**
 * @brief   Get the error handler for an environment.
 *
 * @param[in] env  Environment to query.
 *
 * @return  Pointer to jump buffer, or NULL if not set.
 */
PI_InlineApi(jmp_buf *) piEnvGetErrorHandler(const PiEnv *const env)
{
    return env ? env->errorJmp : NULL;
}

/**
 * @brief   Raise an error in the environment context.
 *
 * If an error handler is set (via piEnvSetErrorHandler), performs longjmp
 * to that handler. Otherwise, calls the global error handler (piRaiseError),
 * which typically terminates the program.
 *
 * @param[in] env     Environment context.
 * @param[in] format  Printf-style format string.
 * @param[in] ...     Format arguments.
 *
 * @note    This function does not return.
 */
PI_NORET PI_Api(void) piEnvRaiseError(PiEnv *const env, const char *const format, ...);

/* =---- Environment I/O API -----------------------------------= */

/**
 * @brief   Get the environment's working directory.
 *
 * Returns the environment-local working directory if set, otherwise
 * uses the I/O handler's fsGetCwd function.
 *
 * @param[in] env  Environment context.
 *
 * @return  Pointer to working directory string, or NULL if not available.
 *          The returned pointer is owned by the environment.
 */
PI_InlineApi(const char *) piEnvGetWorkingDir(const PiEnv *const env)
{
    if (!env)
        return NULL;

    if (env->workingDir)
        return env->workingDir;

    if (env->io.fsGetCwd)
        return env->io.fsGetCwd((PiEnv *)env);

    return NULL;
}
/**
 * @brief   Set the environment's working directory.
 *
 * @param[in,out] env   Environment context.
 * @param[in]     path  New working directory path.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
PI_Api(PiIoResult) piEnvSetWorkingDir(PiEnv *const env, const char *const path);

/**
 * @brief   Open a file using the environment's I/O handlers.
 *
 * @param[in] env    Environment context.
 * @param[in] path   Path to file.
 * @param[in] flags  Open flags (PiStreamFlags).
 *
 * @return  Pointer to stream on success, NULL on failure.
 */
PI_InlineApi(PiStream *) piEnvFileOpen(PiEnv *const env, const char *const path, const PiStreamFlags flags)
{
    return (env && env->io.fileOpen) ? env->io.fileOpen(env, path, flags) : NULL;
}
/**
 * @brief   Close a file using the environment's I/O handlers.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to close.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
PI_InlineApi(PiIoResult) piEnvFileClose(PiEnv *const env, PiStream *const stream)
{
    return (env && env->io.fileClose) ? env->io.fileClose(env, stream) : PI_IO_NOT_SUPPORTED;
}

/**
 * @brief   Read from a file using the environment's I/O handlers.
 *
 * @param[in]  env     Environment context.
 * @param[in]  stream  Stream to read from.
 * @param[out] buf     Buffer to read into.
 * @param[in]  size    Maximum bytes to read.
 *
 * @return  Number of bytes read, or negative error code.
 */
PI_InlineApi(int64_t) piEnvFileRead(PiEnv *const env, PiStream *const stream, void *const buf, const size_t size)
{
    return (env && env->io.fileRead) ? env->io.fileRead(env, stream, buf, size) : PI_IO_NOT_SUPPORTED;
}
/**
 * @brief   Write to a file using the environment's I/O handlers.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to write to.
 * @param[in] data    Data to write.
 * @param[in] size    Number of bytes to write.
 *
 * @return  Number of bytes written, or negative error code.
 */
PI_InlineApi(int64_t) piEnvFileWrite(PiEnv *const env, PiStream *const stream, const void *const data, const size_t size)
{
    return (env && env->io.fileWrite) ? env->io.fileWrite(env, stream, data, size) : PI_IO_NOT_SUPPORTED;
}

/**
 * @brief   Write to standard output using the environment's console handler.
 *
 * @param[in] env   Environment context.
 * @param[in] data  Data to write.
 * @param[in] size  Number of bytes to write.
 *
 * @return  Number of bytes written, or negative error code.
 */
PI_InlineApi(int64_t) piEnvPrint(PiEnv *const env, const void *const data, const size_t size)
{
    return (env && env->io.consoleWrite) ? env->io.consoleWrite(env, 1, data, size) : PI_IO_NOT_SUPPORTED;
}
/**
 * @brief   Write to standard error using the environment's console handler.
 *
 * @param[in] env   Environment context.
 * @param[in] data  Data to write.
 * @param[in] size  Number of bytes to write.
 *
 * @return  Number of bytes written, or negative error code.
 */
PI_InlineApi(int64_t) piEnvPrintErr(PiEnv *const env, const void *const data, const size_t size)
{
    return (env && env->io.consoleWrite) ? env->io.consoleWrite(env, 2, data, size) : PI_IO_NOT_SUPPORTED;
}

/* =---- Environment Capability API ----------------------------= */

/**
 * @brief   Check if an environment has a specific capability.
 *
 * @param[in] env  Environment to check.
 * @param[in] cap  Capability flag to test.
 *
 * @return  true if the capability is granted, false otherwise.
 */
PI_InlineApi(bool) piEnvHasCapability(const PiEnv *const env, const PiCapability cap)
{
    return env && piHasCapability(&env->caps, cap);
}
/**
 * @brief   Get the capability context for an environment.
 *
 * @param[in] env  Environment to query.
 *
 * @return  Pointer to capability context, or NULL if env is NULL.
 */
PI_InlineApi(const PiCapabilityContext *) piEnvGetCapabilities(const PiEnv *const env)
{
    return env ? &env->caps : NULL;
}

/**
 * @brief   Require a capability, raising an error if not present.
 *
 * If the environment does not have the required capability, raises
 * an error that either jumps to the error handler or terminates.
 *
 * @param[in] env   Environment context.
 * @param[in] cap   Required capability.
 * @param[in] what  Description of operation for error message.
 *
 * @return  true if capability is present (only returns if present).
 */
PI_Api(bool) piEnvRequireCapability(PiEnv *const env, const PiCapability cap, const char *const what);

/* =---- Child Environment API ---------------------------------= */

/**
 * @brief   Create a child environment with restricted capabilities.
 *
 * The child environment inherits handlers and configuration from the parent,
 * but can only have capabilities that are a subset of the parent's capabilities.
 *
 * @param[in]  parent    Parent environment.
 * @param[out] child     Child environment structure to initialize.
 * @param[in]  caps      Requested capabilities (will be intersected with parent's).
 * @param[in]  heapSize  Heap size for child (0 for default).
 *
 * @return  Pointer to initialized child environment, or NULL on failure.
 *
 * @note    The parent must have PI_CAP_ENV_SPAWN capability.
 *          The child will have PI_ENV_FLAG_IS_CHILD flag set.
 *
 * @example
 * @code
 *     PiEnv childEnv;
 *     // Create child with only console write capability
 *     piCreateChildEnv(parent, &childEnv, PI_CAP_CONSOLE_WRITE, 0);
 *     // ... use childEnv ...
 *     piFreeEnv(&childEnv);
 * @endcode
 */
PI_Api(PiEnv *) piCreateChildEnv(PiEnv *const parent, PiEnv *const child, const uint32_t caps, const size_t heapSize);

/**
 * @brief   Get the parent environment.
 *
 * @param[in] env  Environment to query.
 *
 * @return  Pointer to parent environment, or NULL if root.
 */
PI_InlineApi(PiEnv *) piEnvGetParent(const PiEnv *const env)
{
    return env ? env->relation.parent : NULL;
}
/**
 * @brief   Check if an environment is a child environment.
 *
 * @param[in] env  Environment to check.
 *
 * @return  true if this is a child environment, false if root.
 */
PI_InlineApi(bool) piEnvIsChild(const PiEnv *const env)
{
    return env && (env->flags & PI_ENV_FLAG_IS_CHILD) != 0;
}
/**
 * @brief   Get the number of child environments.
 *
 * @param[in] env  Environment to query.
 *
 * @return  Number of active child environments.
 */
PI_InlineApi(size_t) piEnvGetChildCount(const PiEnv *const env)
{
    return env ? env->relation.childCount : 0;
}

/* =---- Environment Interrupt API -----------------------------= */

/**
 * @brief   Request an interrupt in the environment.
 *
 * Sets the interrupt flag. Running code should check this periodically
 * and stop execution gracefully.
 *
 * @param[in,out] env  Environment to interrupt.
 */
PI_InlineApi(void) piEnvInterrupt(PiEnv *const env)
{
    if (env)
        env->flags |= PI_ENV_FLAG_INTERRUPT;

    return;
}
/**
 * @brief   Check if an interrupt has been requested.
 *
 * @param[in] env  Environment to check.
 *
 * @return  true if interrupt requested, false otherwise.
 */
PI_InlineApi(bool) piEnvIsInterrupted(const PiEnv *const env)
{
    return env && (env->flags & PI_ENV_FLAG_INTERRUPT) != 0;
}
/**
 * @brief   Clear the interrupt flag.
 *
 * @param[in,out] env  Environment to clear.
 */
PI_InlineApi(void) piEnvClearInterrupt(PiEnv *const env)
{
    if (env)
        env->flags &= ~PI_ENV_FLAG_INTERRUPT;

    return;
}

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
