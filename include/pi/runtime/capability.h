#pragma once

/**
 * @file        capability.h
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
 * @brief       Capability-based security model for environment sandboxing.
 *
 *              Provides a fine-grained permission system that controls what
 *              operations an environment can perform. Child environments can
 *              only have a subset of their parent's capabilities.
 */

#ifndef _PI_RUNTIME_CAPABILITY_H
#define _PI_RUNTIME_CAPABILITY_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Capability Flags --------------------------------------= */

/**
 * @brief   Capability flags for environment permissions.
 *
 * These flags control what operations an environment is allowed to perform.
 * Capabilities are inherited from parent to child, but children can only
 * have a subset of their parent's capabilities.
 *
 * @note    Use PI_CAP_ALL for unrestricted environments (default for root).
 *          Use PI_CAP_NONE for fully sandboxed environments.
 */
typedef enum _pi_Capability
{
    /**
     * @brief   No capabilities (fully sandboxed).
     */
    PI_CAP_NONE             = 0,

    /* ---- Memory Capabilities ---- */

    /**
     * @brief   Can allocate memory from heap.
     */
    PI_CAP_MEM_ALLOC        = PI_BitFlag(1),
    /**
     * @brief   No memory limit enforcement.
     */
    PI_CAP_MEM_UNLIMITED    = PI_BitFlag(2),

    /* ---- File System Capabilities ---- */

    /**
     * @brief   Can read files.
     */
    PI_CAP_FS_READ          = PI_BitFlag(3),
    /**
     * @brief   Can write to existing files.
     */
    PI_CAP_FS_WRITE         = PI_BitFlag(4),
    /**
     * @brief   Can create new files.
     */
    PI_CAP_FS_CREATE        = PI_BitFlag(5),
    /**
     * @brief   Can delete files.
     */
    PI_CAP_FS_DELETE        = PI_BitFlag(6),
    /**
     * @brief   Can create directories.
     */
    PI_CAP_FS_MKDIR         = PI_BitFlag(7),
    /**
     * @brief   Can access paths outside sandbox.
     */
    PI_CAP_FS_ESCAPE        = PI_BitFlag(8),

    /* ---- Console Capabilities ---- */

    /**
     * @brief   Can read from console (stdin).
     */
    PI_CAP_CONSOLE_READ     = PI_BitFlag(9),
    /**
     * @brief   Can write to console (stdout/stderr).
     */
    PI_CAP_CONSOLE_WRITE    = PI_BitFlag(10),

    /* ---- Environment Capabilities ---- */

    /**
     * @brief   Can spawn child environments.
     */
    PI_CAP_ENV_SPAWN        = PI_BitFlag(11),
    /**
     * @brief   Can communicate via channels.
     */
    PI_CAP_ENV_CHANNEL      = PI_BitFlag(12),
    /**
     * @brief   Can access parent environment.
     */
    PI_CAP_ENV_PARENT       = PI_BitFlag(13),

    /* ---- System Capabilities ---- */

    /**
     * @brief   Can access system time.
     */
    PI_CAP_SYS_TIME         = PI_BitFlag(14),
    /**
     * @brief   Can call native C functions.
     */
    PI_CAP_SYS_NATIVE       = PI_BitFlag(15),
    /**
     * @brief   Can access environment variables.
     */
    PI_CAP_SYS_GETENV       = PI_BitFlag(16),
    /**
     * @brief   Can execute external processes.
     */
    PI_CAP_SYS_EXEC         = PI_BitFlag(17),

    /* ---- Convenience Groups ---- */

    /**
     * @brief   All file system capabilities.
     */
    PI_CAP_FS_ALL           = PI_CAP_FS_READ | PI_CAP_FS_WRITE |
                              PI_CAP_FS_CREATE | PI_CAP_FS_DELETE |
                              PI_CAP_FS_MKDIR | PI_CAP_FS_ESCAPE,
    /**
     * @brief   All console capabilities.
     */
    PI_CAP_CONSOLE_ALL      = PI_CAP_CONSOLE_READ | PI_CAP_CONSOLE_WRITE,
    /**
     * @brief   All environment capabilities.
     */
    PI_CAP_ENV_ALL          = PI_CAP_ENV_SPAWN | PI_CAP_ENV_CHANNEL |
                              PI_CAP_ENV_PARENT,
    /**
     * @brief   All system capabilities.
     */
    PI_CAP_SYS_ALL          = PI_CAP_SYS_TIME | PI_CAP_SYS_NATIVE |
                              PI_CAP_SYS_GETENV | PI_CAP_SYS_EXEC,
    /**
     * @brief   All memory capabilities.
     */
    PI_CAP_MEM_ALL          = PI_CAP_MEM_ALLOC | PI_CAP_MEM_UNLIMITED,

    /**
     * @brief   All capabilities (unrestricted).
     */
    PI_CAP_ALL              = 0xFFFFFFFF,
} PiCapability;

/* =---- Capability Context ------------------------------------= */

/**
 * @brief   Capability context for an environment.
 *
 * Contains the set of capabilities and resource limits for an environment.
 * Used to enforce sandboxing and security restrictions.
 */
typedef struct _pi_CapabilityContext
{
    /**
     * @brief   Bitfield of enabled capabilities.
     *
     * @see     PiCapability
     */
    uint32_t            flags;
    /**
     * @brief   Maximum memory that can be allocated (bytes).
     *
     * 0 means unlimited (if PI_CAP_MEM_UNLIMITED is set).
     */
    size_t              memoryLimit;
    /**
     * @brief   Maximum number of open file handles.
     *
     * 0 means unlimited.
     */
    size_t              fileCountLimit;
    /**
     * @brief   Maximum recursion depth for function calls.
     *
     * 0 means unlimited.
     */
    size_t              callDepthLimit;
    /**
     * @brief   Array of allowed path prefixes (whitelist).
     *
     * If non-NULL and allowedPathCount > 0, file operations are restricted
     * to paths that start with one of these prefixes. Only applies if
     * PI_CAP_FS_ESCAPE is NOT set.
     */
    const char *const  *allowedPaths;
    /**
     * @brief   Number of entries in allowedPaths array.
     */
    size_t              allowedPathCount;
} PiCapabilityContext;

/* =---- Capability Check Functions ----------------------------= */

/**
 * @brief   Check if a capability context has a specific capability.
 *
 * @param[in] ctx  Capability context to check.
 * @param[in] cap  Capability flag to test.
 *
 * @return  true if the capability is granted, false otherwise.
 *
 * @example
 * @code
 *     if (piHasCapability(&env->caps, PI_CAP_FS_WRITE)) {
 *         // Can write files
 *     }
 * @endcode
 */
PI_InlineApi(bool) piHasCapability(const PiCapabilityContext *const ctx, const PiCapability cap)
{
    return ctx && (ctx->flags & (uint32_t)cap) == (uint32_t)cap;
}

/**
 * @brief   Check if a capability context has any of the specified capabilities.
 *
 * @param[in] ctx   Capability context to check.
 * @param[in] caps  Bitfield of capabilities to test.
 *
 * @return  true if any of the capabilities are granted, false otherwise.
 */
PI_InlineApi(bool) piHasAnyCapability(const PiCapabilityContext *const ctx, const uint32_t caps)
{
    return ctx && (ctx->flags & caps) != 0;
}

/**
 * @brief   Check if a capability context has all of the specified capabilities.
 *
 * @param[in] ctx   Capability context to check.
 * @param[in] caps  Bitfield of capabilities to test.
 *
 * @return  true if all capabilities are granted, false otherwise.
 */
PI_InlineApi(bool) piHasAllCapabilities(const PiCapabilityContext *const ctx, const uint32_t caps)
{
    return ctx && (ctx->flags & caps) == caps;
}

/**
 * @brief   Grant capabilities to a context.
 *
 * @param[in,out] ctx   Capability context to modify.
 * @param[in]     caps  Capabilities to grant.
 *
 * @note    This does not check if the capabilities are valid for the parent.
 *          Use piCapabilityContextDerive() for safe inheritance.
 */
PI_InlineApi(void) piGrantCapabilities(PiCapabilityContext *const ctx, const uint32_t caps)
{
    if (ctx)
        ctx->flags |= caps;

    return;
}

/**
 * @brief   Revoke capabilities from a context.
 *
 * @param[in,out] ctx   Capability context to modify.
 * @param[in]     caps  Capabilities to revoke.
 */
PI_InlineApi(void) piRevokeCapabilities(PiCapabilityContext *const ctx, const uint32_t caps)
{
    if (ctx)
        ctx->flags &= ~caps;

    return;
}

/* =---- Capability Context Initialization ---------------------= */

/**
 * @brief   Initialize a capability context with default values.
 *
 * Sets all capabilities to PI_CAP_NONE and all limits to 0 (unlimited).
 *
 * @param[out] ctx  Context to initialize.
 */
PI_InlineApi(void) piCapabilityContextInit(PiCapabilityContext *const ctx)
{
    if (!ctx)
        return;

    ctx->flags = PI_CAP_NONE;
    ctx->memoryLimit = 0;
    ctx->fileCountLimit = 0;
    ctx->callDepthLimit = 0;
    ctx->allowedPaths = NULL;
    ctx->allowedPathCount = 0;

    return;
}

/**
 * @brief   Initialize a capability context with all capabilities.
 *
 * Creates an unrestricted context suitable for root environments.
 *
 * @param[out] ctx  Context to initialize.
 */
PI_InlineApi(void) piCapabilityContextInitAll(PiCapabilityContext *const ctx)
{
    if (!ctx)
        return;

    ctx->flags = PI_CAP_ALL;
    ctx->memoryLimit = 0;
    ctx->fileCountLimit = 0;
    ctx->callDepthLimit = 0;
    ctx->allowedPaths = NULL;
    ctx->allowedPathCount = 0;

    return;
}

/**
 * @brief   Derive a child capability context from a parent.
 *
 * Creates a new context that has at most the capabilities of the parent.
 * The child's requested capabilities are intersected with the parent's
 * capabilities to ensure children cannot escalate privileges.
 *
 * @param[out] child           Context to initialize.
 * @param[in]  parent          Parent context to derive from.
 * @param[in]  requestedFlags  Capabilities requested for child.
 *
 * @return  true if derivation succeeded, false if parent is NULL.
 *
 * @example
 * @code
 *     PiCapabilityContext childCaps;
 *     // Request file read but parent only has read permission
 *     piCapabilityContextDerive(&childCaps, &parentCaps,
 *                               PI_CAP_FS_READ | PI_CAP_FS_WRITE);
 *     // childCaps.flags will only have PI_CAP_FS_READ
 * @endcode
 */
PI_InlineApi(bool) piCapabilityContextDerive(PiCapabilityContext *const child, const PiCapabilityContext *const parent, const uint32_t requestedFlags)
{
    if (!child || !parent)
        return false;

    /* Child can only have capabilities that parent has */
    child->flags = parent->flags & requestedFlags;

    /* Inherit limits from parent (child can have stricter but not looser) */
    child->memoryLimit = parent->memoryLimit;
    child->fileCountLimit = parent->fileCountLimit;
    child->callDepthLimit = parent->callDepthLimit;

    /* Inherit path restrictions */
    child->allowedPaths = parent->allowedPaths;
    child->allowedPathCount = parent->allowedPathCount;

    return true;
}

/**
 * @brief   Set stricter limits on a capability context.
 *
 * Only allows setting limits that are more restrictive than current values.
 * If a current limit is unlimited (0), any positive value is more restrictive.
 *
 * @param[in,out] ctx             Context to modify.
 * @param[in]     memoryLimit     New memory limit (0 to keep current).
 * @param[in]     fileCountLimit  New file count limit (0 to keep current).
 * @param[in]     callDepthLimit  New call depth limit (0 to keep current).
 */
PI_InlineApi(void) piCapabilityContextSetLimits(PiCapabilityContext *const ctx, const size_t memoryLimit, const size_t fileCountLimit, const size_t callDepthLimit)
{
    if (!ctx)
        return;

    /* Only set if more restrictive (or current is unlimited) */
    if (memoryLimit > 0 && (ctx->memoryLimit == 0 || memoryLimit < ctx->memoryLimit))
        ctx->memoryLimit = memoryLimit;

    if (fileCountLimit > 0 && (ctx->fileCountLimit == 0 || fileCountLimit < ctx->fileCountLimit))
        ctx->fileCountLimit = fileCountLimit;

    if (callDepthLimit > 0 && (ctx->callDepthLimit == 0 || callDepthLimit < ctx->callDepthLimit))
        ctx->callDepthLimit = callDepthLimit;

    return;
}

/* =---- Path Validation ---------------------------------------= */

/**
 * @brief   Check if a path is allowed by the capability context.
 *
 * If PI_CAP_FS_ESCAPE is set or no allowed paths are configured,
 * all paths are allowed. Otherwise, the path must start with one
 * of the allowed path prefixes.
 *
 * @param[in] ctx   Capability context.
 * @param[in] path  Path to validate.
 *
 * @return  true if the path is allowed, false otherwise.
 *
 * @note    This performs prefix matching only. Callers should normalize
 *          the path first to prevent bypass via ".." or symlinks.
 */
PI_Api(bool) piIsPathAllowed(const PiCapabilityContext *const ctx, const char *const path);

/**
 * @brief   Set the allowed paths for a capability context.
 *
 * @param[in,out] ctx        Context to modify.
 * @param[in]     paths      Array of allowed path prefixes.
 * @param[in]     pathCount  Number of paths in array.
 *
 * @note    The paths array must remain valid for the lifetime of the context.
 *          The context does not take ownership of the array.
 */
PI_InlineApi(void) piCapabilityContextSetPaths(PiCapabilityContext *const ctx, const char *const *const paths, const size_t pathCount)
{
    if (!ctx)
        return;

    ctx->allowedPaths = paths;
    ctx->allowedPathCount = pathCount;

    return;
}

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
