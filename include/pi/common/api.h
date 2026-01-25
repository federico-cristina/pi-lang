#pragma once

/**
 * @file        api.h
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
 * @brief       DLL export/import and calling convention macros for cross-platform library
 *              building.
 * 
 *              This header provides macros for creating shared libraries (DLLs) that work
 *              across Windows, Linux, macOS, and other Unix-like systems with GCC, Clang,
 *              and MSVC compilers.
 * 
 *              Build Modes:
 *               - Define PI_BUILD_DLL when building the shared library (exports symbols)
 *               - Define PI_USE_DLL when using the shared library (imports symbols on Windows)
 *               - Define neither for static library builds (no special attributes)
 *
 *              Usage Example:
 * @code
 *                  PI_Api(void) myFunction(int x, int y);
 *                  PI_Api(int) calculate(double a, double b);
 *                  PI_Api(const char*) getString(void);
 * @endcode
 */

#ifndef _PI_COMMON_API_H
#define _PI_COMMON_API_H

/**
 * +---- Platform Detection ---------------+
 */

#if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__)
/**
 * @brief   Indicates that the target platform is Windows.
 */
#   define PI_PLATFORM_WINDOWS 1
#else
/**
 * @brief   Indicates that the target platform is Unix-like (Linux, macOS, BSD, etc.).
 */
#   define PI_PLATFORM_WINDOWS 0
#endif

/**
 * +---- Calling Convention ---------------+
 */

#ifndef PI_CCONV
/**
 * @brief   Calling convention for exported functions.
 *
 *          On Windows, this is __cdecl (the standard C calling convention).
 *          On Unix-like systems, no special calling convention is needed.
 *
 * @note    This macro is used by PI_Api(T) and can be used independently
 *          for more complex function declarations.
 */
#   if PI_PLATFORM_WINDOWS
#       if defined(_MSC_VER)
#           define PI_CCONV __cdecl
#       elif defined(__GNUC__) || defined(__clang__)
#           define PI_CCONV __cdecl
#       else
#           define PI_CCONV
#       endif
#   else
#       define PI_CCONV
#   endif
#endif

/**
 * +---- DLL Export/Import ----------------+
 */

#ifndef PI_API
/**
 * @brief   Base macro for DLL symbol visibility control.
 *
 *          This macro expands to the appropriate compiler-specific attribute for:
 *           - Exporting symbols when building a DLL (PI_BUILD_DLL defined)
 *           - Importing symbols when using a DLL (PI_USE_DLL defined on Windows)
 *           - No special attributes for static library builds
 *
 * @note    For Unix-like systems, compile with -fvisibility=hidden to make
 *          the visibility attributes effective.
 */
#   if defined(PI_BUILD_DLL)
        /* ===== Building the DLL/shared library ===== */
#       if PI_PLATFORM_WINDOWS
#           define PI_API __declspec(dllexport)
#       else
#           if defined(__GNUC__) || defined(__clang__)
#               define PI_API __attribute__((visibility("default")))
#           else
#               define PI_API
#           endif
#       endif
#   elif defined(PI_USE_DLL)
        /* ===== Using the DLL/shared library ===== */
#       if PI_PLATFORM_WINDOWS
#           define PI_API __declspec(dllimport)
#       else
#           define PI_API
#       endif
#   else
        /* ===== Static library build (default) ===== */
#       define PI_API
#   endif
#endif

/**
 * +---- Inline Specifiers -----------------+
 */

#ifndef PI_INLINE
/**
 * @brief   Declares a function as static inline.
 *
 *          Static inline functions are compiled into each translation unit
 *          that includes them. They do not require DLL export/import.
 */
#   define PI_INLINE static inline
#endif

#ifndef PI_FORCEINLINE
/**
 * @brief   Compiler hint to always inline a function.
 *
 *          This is a stronger hint than PI_INLINE, telling the compiler
 *          to inline the function even when optimizations are disabled.
 *
 * @note    This is only a hint; the compiler may still choose not to inline.
 */
#   if defined(_MSC_VER)
#       define PI_FORCEINLINE static __forceinline
#   elif defined(__GNUC__) || defined(__clang__)
#       define PI_FORCEINLINE static inline __attribute__((always_inline))
#   else
#       define PI_FORCEINLINE static inline
#   endif
#endif

/**
 * +---- Function Declaration Macros -------+
 */

#ifndef PI_Api
/**
 * @brief   Declares a function with return type T that is exported from/imported to the DLL.
 *
 *          This macro combines the DLL visibility attribute (PI_API) and calling convention
 *          (PI_CCONV) into a single, convenient macro for declaring library functions.
 *
 * @param T The return type of the function.
 *
 * @example Basic usage:
 * @code
 *          // In header file:
 *          PI_Api(void) pi_InitSystem(void);
 *          PI_Api(int) pi_Calculate(double x, double y);
 *          PI_Api(const char*) pi_GetVersion(void);
 *
 *          // In implementation file:
 *          PI_Api(void) pi_InitSystem(void) {
 *              // Implementation...
 *          }
 * @endcode
 *
 * @note    Expands to: PI_API ReturnType PI_CCONV
 *          Example: __declspec(dllexport) void __cdecl (on Windows when building DLL)
 */
#   define PI_Api(T) PI_API T PI_CCONV
#endif

#ifndef PI_InlineApi
/**
 * @brief   Declares an inline function with return type T as part of the public API.
 *
 *          This macro is used for inline functions defined in public headers.
 *          These functions are compiled into each translation unit and do not
 *          require DLL symbol export. The macro provides consistency with PI_Api(T)
 *          and marks the function as part of the public API.
 *
 * @param T The return type of the function.
 *
 * @example Basic usage:
 * @code
 *          // In header file:
 *          PI_InlineApi(bool) piHasCapability(const PiCapabilityContext *ctx, PiCapability cap)
 *          {
 *              return ctx && (ctx->flags & (uint32_t)cap) == (uint32_t)cap;
 *          }
 * @endcode
 */
#   define PI_InlineApi(T) PI_API PI_INLINE T PI_CCONV
#endif

/**
 * +---------------------------------------+
 */

#endif
