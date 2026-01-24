#pragma once

/**
 * @file        system.h
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
 * @brief       System initialization, configuration, and platform-specific utilities.
 */

#ifndef _PI_COMMON_SYSTEM_H
#define _PI_COMMON_SYSTEM_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- System Initialization ---------------------------------= */

/**
 * @brief   Initializes the system by enabling platform-specific features.
 *
 *          This function configures the runtime environment, including:
 *          - Enabling ANSI terminal colors on Windows
 *          - Detecting CPU architecture and OS information
 *          - Caching the current working directory
 *          - Determining system memory page size
 *
 * @note    This function should be called exactly once at the beginning of
 *          the main() function. Multiple calls are safe but redundant.
 *
 * @see     piFreeSystem()
 */
PI_Api(void) piInitSystem(void);
/**
 * @brief   Cleans up system resources allocated by piInitSystem().
 *
 *          Frees any memory allocated during system initialization and resets
 *          internal state. After calling this function, piInitSystem() can be
 *          called again to reinitialize.
 *
 * @note    This function should be called exactly once at the end of the
 *          main() function before program termination.
 *
 * @see     piInitSystem()
 */
PI_Api(void) piFreeSystem(void);

/* =---- Platform Utilities ------------------------------------= */

/**
 * @brief   Enables ANSI virtual terminal processing.
 *
 *          On Windows, this enables support for ANSI escape codes in the console,
 *          allowing colored text output and cursor control. On Unix-like systems,
 *          this function has no effect as ANSI codes are supported by default.
 *
 * @note    Called automatically by piInitSystem() when PI_USE_COLORS is enabled.
 *
 * @warning Requires piInitSystem() to be called first in debug builds.
 */
PI_Api(void) piEnableVirtualTerminal(void);

/**
 * @brief   Changes the console window title.
 *
 * @param[in] title  New title string for the console window. Must not be NULL.
 *
 * @note    Uses SetConsoleTitleA() on Windows and ANSI escape sequences on Unix.
 *
 * @warning Requires piInitSystem() to be called first in debug builds.
 */
PI_Api(void) piSetConsoleTitle(const char *const title);
/**
 * @brief   Changes the current working directory of the process.
 *
 * @param[in] path   Path to the new working directory. Must not be NULL.
 *
 * @note    Updates the internal cached directory path maintained by the system.
 *          Use piGetCurrentDirectory() to retrieve the current path.
 *
 * @warning Requires piInitSystem() to be called first in debug builds.
 *          May raise a recoverable error if the directory change fails.
 *
 * @see     piGetCurrentDirectory()
 */
PI_Api(void) piSetCurrentDirectory(const char *const path);

/**
 * @brief   Retrieves the current working directory path.
 *
 * @return  Pointer to a null-terminated string containing the current directory path.
 *          The returned pointer is valid until piSetCurrentDirectory() or piFreeSystem()
 *          is called. Returns NULL if the path could not be determined.
 *
 * @note    The returned string is managed internally and must not be freed or modified.
 *
 * @warning Requires piInitSystem() to be called first in debug builds.
 *
 * @see     piSetCurrentDirectory()
 */
PI_Api(const char *) piGetCurrentDirectory(void);
/**
 * @brief   Returns the system memory page size.
 *
 * @return  Size of a system memory page in bytes (typically 4096 on most platforms).
 *
 * @note    This value is determined during piInitSystem() and cached for performance.
 *          Useful for aligned memory allocations and memory-mapped I/O.
 *
 * @warning Returns 0 if piInitSystem() has not been called.
 */
PI_Api(size_t) piGetPageSize(void);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
