#pragma once

/**
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

#ifndef PI_DEBUG
/**
 * @brief   Represents a debug flag.
 */
#   ifdef _DEBUG
#       define PI_DEBUG 1
#   else
#       define PI_DEBUG 0
#   endif
#endif

/**
 * +---- Compilation Options --------------+
 */

#ifndef PI_USE_COLORS
/**
 * @brief   This option specifies when to enable colored text output.
 * 
 * @note    This option relies on ANSI color escape codes. On Windows, terminal colors
 *          are not enabled by default, so they are enabled during system configuration.
 */
#   define PI_USE_COLORS                   1
#endif

#ifndef PI_USE_BOXED_VALUES
/**
 * @brief   This option determines whether values ​​and value registers are stored in a
 *          compressed manner (normally used in NaN-boxing to reduce their size, but more
 *          precise techniques may be implemented depending on the system).
 * 
 * @note    This option refers primarily to the `PiValue` data type, but is defined here
 *          for neatness reasons and because other components of the program might base
 *          some compilation differences on this option.
 */
#   define PI_USE_BOXED_VALUES             1
#endif

/**
 * +---- C++ Compilation Support ----------+
 */

#ifndef PI_C_HEADER_BEGIN
/**
 * @brief   Indicates the beginning of a C header for C++ compilation.
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
 * @brief   Indicates the ending of a C header for C++ compilation.
 */
#   if   defined _CRT_BEGIN_C_HEADER
#       define PI_C_HEADER_END _CRT_END_C_HEADER
#   elif defined __cplusplus
#       define PI_C_HEADER_END }
#   else
#       define PI_C_HEADER_END
#   endif
#endif

/**
 * +---- Other Definitions ----------------+
 */

#ifndef PI_NORET
/**
 * @brief   This macro marks functions that do not return.
 */
#   if   defined _MSC_VER
#       define PI_NORET __declspec(noreturn)
#   elif defined __GNUC__
#       define PI_NORET __attribute__((noreturn))
#   else
#       define PI_NORET
#   endif
#endif

/**
 * +---------------------------------------+
 */

/* Include minsc definitions */
#include <assert.h>
#include <limits.h>
#include <float.h>

/* Include definitions for signal handling and fwd jumps */
#include <setjmp.h>
#include <signal.h>

/* Include C99 useful data types */
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* Include C99 fixed-width int types PRIs */
#include <inttypes.h>

/* Include text coloring macros */
#include "pi/common/colors.h"

PI_C_HEADER_BEGIN

/* =---- System ------------------------------------------------= */

/**
 * @brief   This function initializes the system by enabling any disabled settings (such
 *          as colors in the Windows terminal) and obtains useful information for performing
 *          specific system operations.
 * 
 * @note    This function should be called only once at the beginning of the `main` function
 *          and that's it.
 */
void pi_InitSystem(void);
/**
 * @brief   This function undoes everything that `pi_InitSystem` did.
 * 
 * @note    This function should be called only once at the end of the `main` function and
 *          that's it.
 */
void pi_FreeSystem(void);

/**
 * +---- System Management ----------------+
 */

void piEnableVirtualTerminal(void);

/**
 * @brief   This function changes the console title.
 * 
 * @param[in] title The title the console window will have after the call to this function. 
 */
void piSetConsoleTitle(const char *const title);
/**
 * @brief   This function changes the current working directory of the process.
 * 
 * @param[in] path  The path to the new cwd. 
 */
void piSetCurrentDirectory(const char *const path);

/**
 * @brief   This function returns the current working directory of the process. 
 */
const char *piGetCurrentDirectory(void);
/**
 * @brief   This function returns the size of a system page of memory. 
 */
size_t pi_GetPageSize(void);

/**
 * +---- Internal Errors Handling ---------+
 */

/**
 * @brief   Title for suppressed error messages.
 */
#define PI_TITLE_SUPPRESSED     PI_BOLD_GRAY "suppressed" PI_RESET
/**
 * @brief   Title for annotation messages.
 */
#define PI_TITLE_NOTE           PI_BOLD_CYAN "note" PI_RESET
/**
 * @brief   Title for info-dumping/debugging messages and logs.
 */
#define PI_TITLE_INFO           PI_DARK_CYAN "info" PI_RESET
/**
 * @brief   Title for warning messages.
 */
#define PI_TITLE_WARNING        PI_BOLD_MAGENTA "warning" PI_RESET
/**
 * @brief   Title for error messages.
 */
#define PI_TITLE_ERROR          PI_BOLD_RED "error" PI_RESET
/**
 * @brief   Title for fatal error messages.
 */
#define PI_TITLE_FATAL_ERROR    PI_DARK_RED "fatal error" PI_RESET

/**
 * @brief   Prints a warning message.
 * 
 * @param[in] func      The name of the function that caused the fatal error.
 * @param[in] file      The name of the source file that caused the fatal error.
 * @param     line      The number of the line on which the call was made to this function.
 * @param[in] format    Pointer to a string specifying how to interpret the data.
 * @param     ...       Additional arguments, depending on the format string.
 */
void piWarningAt(const char *const func, const char *const file, const int line, const char *const format, ...);
/**
 * @brief   Prints an error message.
 * 
 * @param[in] func      The name of the function that caused the fatal error.
 * @param[in] file      The name of the source file that caused the fatal error.
 * @param     line      The number of the line on which the call was made to this function.
 * @param[in] format    Pointer to a string specifying how to interpret the data.
 * @param     ...       Additional arguments, depending on the format string.
 */
void piErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);
/**
 * @brief   Prints a fatal error message and aborts the program.
 * 
 * @param[in] func      The name of the function that caused the fatal error.
 * @param[in] file      The name of the source file that caused the fatal error.
 * @param     line      The number of the line on which the call was made to this function.
 * @param[in] format    Pointer to a string specifying how to interpret the data.
 * @param     ...       Additional arguments, depending on the format string.
 * 
 * @note    This function does not return.
 */
PI_NORET void piFatalErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);

#ifndef piWarning
/**
 * @brief   Prints a warning message.
 */
#   define piWarning(format, ...) \
    (piWarningAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__), 0)
#endif

#ifndef piError
/**
 * @brief   Prints an error message.
 */
#   define piError(format, ...) \
    (piErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__), 0)
#endif

#ifndef piFatal
/**
 * @brief   Prints a fatal error message and aborts the program.
 */
#   define piFatal(format, ...) \
    (piFatalErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__), 0)
#endif

#ifndef piUnreachable
/**
 * @brief   Marks unreachable code and aborts the program if reached.
 */
#   define piUnreachable() \
    piFatal("unreachable code has been reached", NULL)
#endif

#ifndef piNotImpl
/**
 * @brief   Marks not implemented code and aborts the program if reached.
 */
#   define piNotImpl() \
    piFatal("not implemented yet", NULL)
#endif

/**
 * @brief   Sets which portion of code will handle raisen errors by `piRaiseErrorAt`
 *          function.
 * 
 * @param[in] buf   A pointer to the istance of `jmp_buf` type in which are stored information
 *                  to perform a long jump.
 * 
 * @note    This mechanism of errors handling is specifically designed to deal with internal
 *          recoverable errors.
 */
void pi_SetErrorHandler(jmp_buf *const buf);

/**
 * @brief   Prints an error message and performs a long jump to the most recent error handler.
 * 
 * @param[in] func      The name of the function that caused the fatal error.
 * @param[in] file      The name of the source file that caused the fatal error.
 * @param     line      The number of the line on which the call was made to this function.
 * @param[in] format    Pointer to a string specifying how to interpret the data.
 * @param     ...       Additional arguments, depending on the format string.
 * 
 * @note    This function does not return.
 * 
 *          If no error handler has been set with `pi_SetErrorHandler` function then this function
 *          calls `abort` to kill the program execution.
 */
PI_NORET void piRaiseErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);

#ifndef piRaiseError
/**
 * @brief   Prints an error message and performs a long jump to the most recent error handler.
 */
#   define piRaiseError(format, ...) \
    (piRaiseErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__), 0)
#endif

/**
 * +---- Internal Memory Management -------+
 */

#ifndef PI_BitsOf
/**
 * @brief   This macro calculates the bit width of a type or value.
 */
#   define PI_BitsOf(x) (sizeof(x) * CHAR_BIT)
#endif

#ifndef PI_CountOf
/**
 * @brief   This macro calculates the number of elements in an allocated array at
 *          compile time.
 */
#   define PI_CountOf(array) (sizeof(array) / sizeof(*(array)))
#endif

/**
 * @brief   Allocates a block of memory of the specified size. When allocation fails,
 *          it raises a fatal error.
 * 
 * @param size  The size of the memory block to allocate, in bytes.
 * 
 * @return  A pointer to the allocated memory block.
 */
void *pi_malloc(const size_t size);
/**
 * @brief   Allocates a block of memory for an array of elements, initializing all bytes
 *          to zero. When allocation fails, it raises a fatal error.
 * 
 * @param count The number of elements to allocate.
 * @param size  The size of each element, in bytes.
 * 
 * @return  A pointer to the allocated memory block.
 */
void *pi_calloc(const size_t count, const size_t size);

#ifndef piNew
/**
 * @brief   Allocates memory for a single instance of the specified type.
 */
#   define piNew(type) \
    ((type *)pi_malloc(sizeof(type)))
#endif

#ifndef piNewArray
/**
 * @brief   Allocates memory for an array of instances of the specified type.
 */
#   define piNewArray(type, count) \
    ((type *)pi_calloc((count), sizeof(type)))
#endif

/**
 * @brief   Reallocates a previously allocated memory block to a new size. When reallocation
 *          fails, it raises a fatal error.
 * 
 * @param block     Pointer to the previously allocated memory block.
 * @param newSize   The new size for the memory block, in bytes.
 * 
 * @return  A pointer to the reallocated memory block.
 */
void *pi_realloc(void *const block, const size_t newSize);
/**
 * @brief   Resizes a memory block, handling allocation and deallocation as needed.
 * 
 * @param block     Pointer to the previously allocated memory block.
 * @param oldSize   The current size of the memory block, in bytes.
 * @param newSize   The new size for the memory block, in bytes.
 * 
 * @return  A pointer to the resized memory block, or NULL if the new size is zero.
 */
void *pi_resize(void *const block, const size_t oldSize, const size_t newSize);

#ifndef piResize
/**
 * @brief   Resizes a memory block, handling allocation and deallocation as needed.
 */
#   define piResize(type, block, oldSize, newSize) \
    ((type *)pi_resize((void *)(block), sizeof(type) * (oldSize), sizeof(type) * (newSize)))
#endif

/**
 * +---------------------------------------+
 */

#ifndef PI_HasFlag
/**
 * @brief   Check if a binary flagged value has a specific flag set.
 */
#   define PI_HasFlag(x, flag) \
    (((x) & (flag)) != 0)
#endif

#ifndef PI_ShouldGrow
/**
 * @brief   Check if there is enough space for `n` more elements.
 */
#   define PI_ShouldGrow(cap, count, n) \
    ((cap) < ((count) + (n)))
#endif

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
