#pragma once

/**
 * @file        common.h
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
 * @brief       Core common utilities, platform detection, and foundational macros.
 */

#ifndef _PI_COMMON_COMMON_H
#define _PI_COMMON_COMMON_H

#ifndef _CRT_SECURE_NO_WARNINGS
/**
 * @brief   Disables unsafe C function deprecation errors according to CRT checks.
 */
#   define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stddef.h>

/* =---- Debug Configuration -----------------------------------= */

#ifndef PI_DEBUG
/**
 * @brief   Debug build flag (1 for debug, 0 for release).
 *
 * @note    Automatically derived from _DEBUG if not explicitly defined.
 */
#   ifdef _DEBUG
#       define PI_DEBUG 1
#   else
#       define PI_DEBUG 0
#   endif
#endif

/* =---- Compilation Options -----------------------------------= */

#ifndef PI_USE_COLORS
/**
 * @brief   Enable ANSI color codes in terminal output.
 *
 * @note    On Windows, virtual terminal processing must be enabled.
 *          This is done automatically by piInitSystem() when this flag is set.
 */
#   define PI_USE_COLORS 1
#endif

#ifndef PI_USE_BOXED_VALUES
/**
 * @brief   Enable value compression using NaN-boxing or similar techniques.
 *
 * @note    Refers primarily to the PiValue data type. When enabled, values
 *          are stored in a compressed format to reduce memory usage.
 */
#   define PI_USE_BOXED_VALUES 1
#endif

#ifndef PI_USE_JUMP_TABLE
/**
 * @brief   Enable computed goto (direct threading) for VM dispatch optimization.
 *
 * @note    Requires GCC or Clang compiler support (uses &&label extension).
 *          CMake automatically detects support and sets this flag accordingly.
 *          Provides 15-25% performance improvement in the Stack VM.
 */
#   define PI_USE_JUMP_TABLE 0
#endif

/* =---- C++ Compatibility -------------------------------------= */

#ifndef PI_C_HEADER_BEGIN
/**
 * @brief   Marks the beginning of a C header for C++ compilation.
 */
#   if   defined _CRT_BEGIN_C_HEADER
#       define PI_C_HEADER_BEGIN _CRT_BEGIN_C_HEADER
#   elif defined __cplusplus
#       define PI_C_HEADER_BEGIN extern "C" {
#   else
#       define PI_C_HEADER_BEGIN
#   endif
#endif

#ifndef PI_C_HEADER_END
/**
 * @brief   Marks the end of a C header for C++ compilation.
 */
#   if   defined _CRT_BEGIN_C_HEADER
#       define PI_C_HEADER_END _CRT_END_C_HEADER
#   elif defined __cplusplus
#       define PI_C_HEADER_END }
#   else
#       define PI_C_HEADER_END
#   endif
#endif

/* =---- Compiler Attributes -----------------------------------= */

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
/* C11 standard noreturn support */
#   include <stdnoreturn.h>
#endif

#ifndef PI_NORET
/**
 * @brief   Marks functions that never return to the caller.
 *
 * @note    Uses C11 standard noreturn when available, falls back to
 *          compiler-specific attributes for older standards.
 */
#   if defined(noreturn)
        /* C11 standard noreturn macro */
#       define PI_NORET noreturn
#   elif defined(_MSC_VER)
        /* MSVC-specific attribute */
#       define PI_NORET __declspec(noreturn)
#   elif defined(__GNUC__)
        /* GCC/Clang attribute */
#       define PI_NORET __attribute__((noreturn))
#   else
        /* No support for noreturn */
#       define PI_NORET
#   endif
#endif

#ifndef PI_LIKELY
/**
 * @brief   Branch prediction hint: condition is likely to be true.
 *
 * @param x  Condition expression to evaluate.
 *
 * @note    Helps the compiler optimize branch prediction. Use for hot paths
 *          where the condition is almost always true (>90% of the time).
 *
 * @example
 * @code
 *     if (PI_LIKELY(ptr != NULL)) {
 *         // Common case: ptr is valid
 *     }
 * @endcode
 */
#   if defined(__GNUC__) || defined(__clang__)
#       define PI_LIKELY(x) __builtin_expect(!!(x), 1)
#   else
#       define PI_LIKELY(x) (x)
#   endif
#endif

#ifndef PI_UNLIKELY
/**
 * @brief   Branch prediction hint: condition is unlikely to be true.
 *
 * @param x  Condition expression to evaluate.
 *
 * @note    Helps the compiler optimize branch prediction. Use for error paths
 *          or rare conditions (<10% of the time).
 *
 * @example
 * @code
 *     if (PI_UNLIKELY(error != 0)) {
 *         // Error handling (rare case)
 *     }
 * @endcode
 */
#   if defined(__GNUC__) || defined(__clang__)
#       define PI_UNLIKELY(x) __builtin_expect(!!(x), 0)
#   else
#       define PI_UNLIKELY(x) (x)
#   endif
#endif

#ifndef PI_THREAD_LOCAL
/**
 * @brief   Thread-local storage specifier for platform-independent code.
 *
 * @note    Uses C11 _Thread_local when available, falls back to
 *          compiler-specific extensions for MSVC (__declspec(thread))
 *          and GCC/Clang (__thread). If not supported, expands to nothing
 *          and emits a warning that thread-safety is compromised.
 *
 * @example
 * @code
 *     static PI_THREAD_LOCAL int threadLocalCounter = 0;
 * @endcode
 */
#   if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
        /* C11 thread-local storage */
#       define PI_THREAD_LOCAL _Thread_local
#   elif defined(_MSC_VER)
        /* MSVC thread-local storage */
#       define PI_THREAD_LOCAL __declspec(thread)
#   elif defined(__GNUC__) || defined(__clang__)
        /* GCC/Clang thread-local storage */
#       define PI_THREAD_LOCAL __thread
#   else
        /* No thread-local storage - not thread-safe */
#       define PI_THREAD_LOCAL
#       warning "Thread-local storage not supported, using global storage (not thread-safe)"
#   endif
#endif

/* =---- Platform-Specific Constants ---------------------------= */

#ifndef PI_CACHE_LINE_SIZE
/**
 * @brief   CPU cache line size for memory alignment optimizations.
 *
 * @note    Platform-specific values:
 *          - x86/x64/ARM (most): 64 bytes
 *          - PowerPC64: 128 bytes
 *          - IBM z/Architecture: 256 bytes
 */
#   if defined(__s390x__)
        /* IBM z/Architecture has 256-byte cache lines */
#       define PI_CACHE_LINE_SIZE 256
#   elif defined(__powerpc64__)
        /* PowerPC 64-bit has 128-byte cache lines */
#       define PI_CACHE_LINE_SIZE 128
#   else
        /* x86, x64, ARM: standard 64-byte cache lines */
#       define PI_CACHE_LINE_SIZE 64
#   endif
#endif

/* =---- Standard Library Includes -----------------------------= */

/* Fundamental C headers */
#include <assert.h>
#include <limits.h>
#include <float.h>

/* Signal handling and non-local jumps */
#include <setjmp.h>
#include <signal.h>

/* C99 standard types */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* C99 fixed-width integer format macros */
#include <inttypes.h>

/* =---- PI-Specific Headers -----------------------------------= */

/* DLL export/import macros and calling conventions */
#include "pi/common/api.h"
/* ANSI color code macros for terminal output */
#include "pi/common/colors.h"
/* System initialization and platform utilities */
#include "pi/common/system.h"
/* Error reporting and recoverable exception handling */
#include "pi/common/error.h"
/* Memory allocation wrappers with safety checks */
#include "pi/common/memory.h"
/* Arena allocator for batch allocations */
#include "pi/common/arena.h"
/* Internal strings manipulation and hashing */
#include "pi/common/string.h"
/* General-purpose hash tables */
#include "pi/common/hash.h"

/* =------------------------------------------------------------= */

#endif
