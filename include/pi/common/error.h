#pragma once

/**
 * @file        error.h
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
 * @brief       Error reporting and handling system with recoverable exceptions.
 */

#ifndef _PI_COMMON_ERROR_H
#define _PI_COMMON_ERROR_H

/* Common defintions (and <setjmp.h> inclusion) */
#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Error Title Constants ---------------------------------= */

/**
 * @brief   Title prefix for suppressed diagnostic messages.
 */
#define PI_TITLE_SUPPRESSED     PI_BOLD_GRAY "suppressed" PI_RESET
/**
 * @brief   Title prefix for informational annotations.
 */
#define PI_TITLE_NOTE           PI_BOLD_CYAN "note" PI_RESET
/**
 * @brief   Title prefix for debugging and diagnostic information.
 */
#define PI_TITLE_INFO           PI_DARK_CYAN "info" PI_RESET
/**
 * @brief   Title prefix for warning messages (non-fatal issues).
 */
#define PI_TITLE_WARNING        PI_BOLD_MAGENTA "warning" PI_RESET
/**
 * @brief   Title prefix for error messages (serious issues).
 */
#define PI_TITLE_ERROR          PI_BOLD_RED "error" PI_RESET
/**
 * @brief   Title prefix for fatal error messages (program termination).
 */
#define PI_TITLE_FATAL_ERROR    PI_DARK_RED "fatal error" PI_RESET

/* =---- Error Reporting Functions -----------------------------= */

/**
 * @brief   Prints a warning message to stderr with source location.
 *
 * @param[in] func    Name of the function where the warning occurred.
 * @param[in] file    Source file name where the warning occurred.
 * @param     line    Line number in the source file.
 * @param[in] format  Printf-style format string for the warning message.
 * @param     ...     Arguments corresponding to the format string.
 *
 * @note    Do not call this function directly. Use the piWarning() macro instead.
 *
 * @see     piWarning()
 */
PI_Api(void) piWarningAt(const char *const func, const char *const file, const int line, const char *const format, ...);

/**
 * @brief   Prints an error message to stderr with source location.
 *
 * @param[in] func    Name of the function where the error occurred.
 * @param[in] file    Source file name where the error occurred.
 * @param     line    Line number in the source file.
 * @param[in] format  Printf-style format string for the error message.
 * @param     ...     Arguments corresponding to the format string.
 *
 * @note    Do not call this function directly. Use the piError() macro instead.
 *
 * @see     piError()
 */
PI_Api(void) piErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);

/**
 * @brief   Prints a fatal error message and terminates the program.
 *
 * @param[in] func    Name of the function where the fatal error occurred.
 * @param[in] file    Source file name where the fatal error occurred.
 * @param     line    Line number in the source file.
 * @param[in] format  Printf-style format string for the error message.
 * @param     ...     Arguments corresponding to the format string.
 *
 * @note    This function does not return. It calls abort() after printing the message.
 *          Do not call this function directly. Use the piFatal() macro instead.
 *
 * @see     piFatal(), piUnreachable(), piNotImpl()
 */
PI_NORET PI_Api(void) piFatalErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);

/**
 * @brief   Prints a recoverable error message and performs a non-local jump.
 *
 * @param[in] func    Name of the function where the error occurred.
 * @param[in] file    Source file name where the error occurred.
 * @param     line    Line number in the source file.
 * @param[in] format  Printf-style format string for the error message.
 * @param     ...     Arguments corresponding to the format string.
 *
 * @note    This function does not return normally. It performs a longjmp() to the
 *          most recently registered error handler via piSetErrorHandler(). If no
 *          error handler is registered, calls abort() instead.
 *
 *          Do not call this function directly. Use the piRaiseError() macro instead.
 *
 * @warning This mechanism is not thread-safe in C89/C99 builds. Use thread-local
 *          storage in C11+ builds for thread safety.
 *
 * @see     piRaiseError(), piSetErrorHandler()
 */
PI_NORET PI_Api(void) piRaiseErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...);

/* =---- Error Reporting Macros --------------------------------= */

#ifndef piWarning
/**
 * @brief   Reports a warning message with automatic source location capture.
 *
 * @param format  Printf-style format string.
 * @param ...     Arguments for the format string.
 *
 * @note    Automatically captures __func__, __FILE__, and __LINE__.
 *
 * @example
 * @code
 *     piWarning("deprecated function called: %s", funcName);
 * @endcode
 */
#   define piWarning(format, ...) \
    piWarningAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__)
#endif

#ifndef piError
/**
 * @brief   Reports an error message with automatic source location capture.
 *
 * @param format  Printf-style format string.
 * @param ...     Arguments for the format string.
 *
 * @note    Automatically captures __func__, __FILE__, and __LINE__.
 *
 * @example
 * @code
 *     piError("invalid argument: expected %d, got %d", expected, actual);
 * @endcode
 */
#   define piError(format, ...) \
    piErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__)
#endif

#ifndef piFatal
/**
 * @brief   Reports a fatal error and terminates the program.
 *
 * @param format  Printf-style format string.
 * @param ...     Arguments for the format string.
 *
 * @note    This macro does not return. Automatically captures source location.
 *
 * @example
 * @code
 *     piFatal("critical resource unavailable: %s", resource);
 * @endcode
 */
#   define piFatal(format, ...) \
    piFatalErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__)
#endif

#ifndef piUnreachable
/**
 * @brief   Marks code that should never be executed.
 *
 *          If this code is reached, prints a fatal error and terminates.
 *          Useful for marking logically impossible branches in switch statements
 *          or after functions that should never return.
 *
 * @note    This macro does not return.
 *
 * @example
 * @code
 *     switch (state) {
 *         case STATE_INIT:   handleInit(); break;
 *         case STATE_ACTIVE: handleActive(); break;
 *         default: piUnreachable();
 *     }
 * @endcode
 */
#   define piUnreachable() \
    piFatal("unreachable code has been reached", NULL)
#endif

#ifndef piNotImpl
/**
 * @brief   Marks code that is not yet implemented.
 *
 *          If this code is reached, prints a fatal error and terminates.
 *          Useful during development to mark placeholder functions.
 *
 * @note    This macro does not return.
 *
 * @example
 * @code
 *     void futureFeature(void) {
 *         piNotImpl();
 *     }
 * @endcode
 */
#   define piNotImpl() \
    piFatal("not implemented yet", NULL)
#endif

#ifndef piRaiseError
/**
 * @brief   Raises a recoverable error that can be caught by an error handler.
 *
 * @param format  Printf-style format string.
 * @param ...     Arguments for the format string.
 *
 * @note    This macro does not return normally. It performs a longjmp() to the
 *          registered error handler. If no handler is registered, terminates.
 *
 * @example
 * @code
 *     jmp_buf handler;
 * 
 *     if (setjmp(handler) == 0) {
 *         piSetErrorHandler(&handler);
 *         piRaiseError("something went wrong: %s", reason);
 *     } else {
 *         // Error caught here
 *     }
 * @endcode
 *
 * @see     piSetErrorHandler()
 */
#   define piRaiseError(format, ...) \
    piRaiseErrorAt(__func__, __FILE__, __LINE__, format, __VA_ARGS__)
#endif

/* =---- Recoverable Error Handling ----------------------------= */

/**
 * @brief   Registers an error handler for recoverable errors.
 *
 * @param[in] buf  Pointer to a jmp_buf where execution will resume when
 *                 piRaiseError() is called. Must not be NULL.
 *
 * @note    This mechanism allows catching and recovering from errors using
 *          setjmp/longjmp. The error handler is thread-local in C11+ builds
 *          with thread-local storage support.
 *
 * @warning Not exception-safe in C++ code. The jmp_buf must remain valid
 *          for the entire duration it is registered.
 *
 * @example
 * @code
 *     jmp_buf errorHandler;
 * 
 *     if (setjmp(errorHandler) == 0) {
 *         piSetErrorHandler(&errorHandler);
 *         // Code that may raise errors
 *         riskyOperation();
 *     } else {
 *         // Error handling code
 *         fprintf(stderr, "Caught error\n");
 *     }
 * 
 *     piSetErrorHandler(NULL); // Clear handler
 * @endcode
 *
 * @see     piRaiseError(), piRaiseErrorAt()
 */
PI_Api(void) piSetErrorHandler(jmp_buf *const buf);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
