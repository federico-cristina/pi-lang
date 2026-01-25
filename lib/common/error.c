#include "pi/common/error.h"
#include "pi/common/colors.h"

#include <stdarg.h>
#include <stdio.h>

/* =---- Internal Utilities ------------------------------------= */

/**
 * @brief   Prints formatted output to stderr.
 */
static inline int veprintf(const char *const format, va_list argList)
{
    return vfprintf(stderr, format, argList);
}

/**
 * @brief   Prints formatted output to stderr.
 */
static inline int eprintf(const char *const format, ...)
{
    int result;
    va_list argList;

    va_start(argList, format);
    result = veprintf(format, argList);
    va_end(argList);

    return result;
}

/**
 * @brief   Prints internal error trace (function, file, and line).
 */
static inline void pi_PrintErrorTrace(const char *const func, const char *const file, const int line)
{
    eprintf("\n  ");

    if (func)
        eprintf("at " PI_YELLOW "%s" PI_RESET "() ", func);

    if (file)
        eprintf("in " PI_WHITE "%s:%d" PI_RESET, file, line);
    else
        eprintf("in unspecified file and line");

    eprintf("\n");

    return;
}

/**
 * @brief   Prints an error message with a given title.
 */
static inline void pi_ErrorAtV(const char *const func, const char *const file, const int line, const char *const title, const char *const format, va_list argList)
{
    if (title)
        eprintf("%s: ", title);

    veprintf(format, argList);
    pi_PrintErrorTrace(func, file, line);

    return;
}

/* =---- Error Reporting Functions -----------------------------= */

PI_Api(void) piWarningAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;

    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_WARNING, format, argList);
    va_end(argList);

    return;
}

PI_Api(void) piErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;

    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_ERROR, format, argList);
    va_end(argList);

    return;
}

PI_Api(void) piErrorAtV(const char *const func, const char *const file, const int line, const char *const format, va_list argList)
{
    pi_ErrorAtV(func, file, line, PI_TITLE_ERROR, format, argList);

    return;
}

PI_NORET PI_Api(void) piFatalErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;

    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_FATAL_ERROR, format, argList);
    va_end(argList);

    abort();
}

/* =---- Recoverable Error Handling ----------------------------= */

/**
 * @brief   Error handler context.
 */
typedef struct _pi_ErrorHandler
{
    jmp_buf *buf;
} PiErrorHandler;

/**
 * @brief   Thread-local error handler.
 */
static PI_THREAD_LOCAL PiErrorHandler g_ErrorHandler = {NULL};

PI_Api(void) piSetErrorHandler(jmp_buf *const buf)
{
    /* Allow NULL to clear g_ErrorHandler.buf */
    g_ErrorHandler.buf = buf;

    return;
}

PI_NORET PI_Api(void) piRaiseErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    const char *title;

    /* Determine title based on handler availability */
    if (g_ErrorHandler.buf)
        title = PI_BOLD PI_RED "error" PI_RESET;
    else
        title = PI_BOLD PI_DARK_RED "fatal error" PI_RESET;

    va_list argList;

    va_start(argList, format);
    pi_ErrorAtV(func, file, line, title, format, argList);
    va_end(argList);

    if (g_ErrorHandler.buf)
        /* Jump to error handler */
        longjmp(*(g_ErrorHandler.buf), (errno ? errno : EXIT_FAILURE));
    else
        abort();
}

/* =------------------------------------------------------------= */
