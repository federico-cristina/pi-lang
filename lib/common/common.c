#include "pi/common/common.h"

#include <stdarg.h>
#include <stdio.h>

#ifdef _WIN32
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

#include <errno.h>

/**
 * @brief   Prints formatted output to standard error stream.
 */
static inline int veprintf(const char *const format, va_list argList)
{
    return vfprintf(stderr, format, argList);
}

/**
 * @brief   Prints formatted output to standard error stream.
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

/* =---- System ------------------------------------------------= */

/**
 * @brief   Represents a record of global information about the system status.
 */
typedef struct _pi_SystemInfo
{
    /**
     * @brief   When this flag is `true` it means that this struct has alredy been
     *          initialized.
     */
    bool                    isInit;
    /**
     * @brief   Represents the page size and the granularity of page protection and
     *          commitment.
     */
    size_t                  pageSize;
    /**
     * @brief   A string representing the processor architecture of the installed operating
     *          system.
     */
    const char             *cpuName;
    /**
     * @brief   A string representing the operating system kind.
     */
    const char             *osName;
    /**
     * @brief   A string representing the path to the current process directory.
     */
    char                   *currentDir;
} PiSystemInfo;

/**
 * @brief   Global system information.
 */
static PiSystemInfo pi_SystemInfo;

#ifdef _WIN32
/**
 * @brief   Gets a string representing the CPU architecture using WinAPI.
 */
static inline const char *pi_GetArchitectureName(const WORD wProcessorArchitecture)
{
    const char *result;

    switch (wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_ALPHA64:
        result = "AXP64";
        break;
    case PROCESSOR_ARCHITECTURE_ALPHA:
        result = "AXP32";
        break;
    case PROCESSOR_ARCHITECTURE_AMD64:
        result = "AMD64";
        break;
    case PROCESSOR_ARCHITECTURE_ARM:
    case PROCESSOR_ARCHITECTURE_ARM32_ON_WIN64:
        result = "ARM32";
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        result = "ARM64";
        break;
    case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64:
    case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
        result = "IA-32";
        break;
    case PROCESSOR_ARCHITECTURE_IA64:
        result = "IA-64";
        break;

    default:
        result = "unkown";
        break;
    }

    return result;
}
#endif

void pi_InitSystem(void)
{
    if (pi_SystemInfo.isInit)
        return;

    /* Marks `pi_SystemInfo` as initialized */
    pi_SystemInfo.isInit = true;

#ifdef _WIN32
#   ifdef _WIN64
    /* Sets OS name to 'Win64' */
    pi_SystemInfo.osName = "Win64";
#   else
    /* Sets OS name to 'Win32' */
    pi_SystemInfo.osName = "Win32";
#   endif

    SYSTEM_INFO sysInfo;

    /* Retrieves system infos */
    GetSystemInfo(&sysInfo);

    /* Sets system page size */
    pi_SystemInfo.pageSize = (size_t)sysInfo.dwPageSize;
    /* Sets processor architecture name */
    pi_SystemInfo.cpuName = pi_GetArchitectureName(sysInfo.wProcessorArchitecture);

    const DWORD currentDirSize = GetCurrentDirectory(0, NULL);

    if (currentDirSize > 0)
    {
        TCHAR *const currentDir = (TCHAR *)calloc(sizeof(TCHAR), MAX_PATH);

        /* Error: Cannot allocate memory?! */
        if (!currentDir)
            piFatal("cannot allocate memory", NULL);

        /* Retrieves current working directory path */
        GetCurrentDirectory(currentDirSize, currentDir);
        /* Sets the current working directory buffer */
        pi_SystemInfo.currentDir = (char *)currentDir;
    }
    else
    {
        /* Error: Path to current working directory is uknown */
        pi_SystemInfo.currentDir = NULL;
    }
#else
    /* Sets system page size */
    pi_SystemInfo.pageSize = sysconf(_SC_PAGESIZE);
#endif

    return;
}

void pi_FreeSystem(void)
{
    if (!pi_SystemInfo.isInit)
        return;

    /* Marks `pi_SystemInfo` as NOT initialized */
    pi_SystemInfo.isInit = false;

    if (pi_SystemInfo.currentDir)
        free((void *)pi_SystemInfo.currentDir);

    /* Resets fields */
    pi_SystemInfo.pageSize = 0;
    pi_SystemInfo.cpuName = NULL;
    pi_SystemInfo.osName = NULL;
    pi_SystemInfo.currentDir = NULL;

    return;
}

/**
 * +---- System Management ----------------+
 */

void piEnableVirtualTerminal(void)
{
    if (PI_DEBUG && !pi_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    const HANDLE
        /* Standard input stream handle */
        stdIn = GetStdHandle(STD_INPUT_HANDLE),
        /* Standard output stream handle */
        stdOut = GetStdHandle(STD_OUTPUT_HANDLE),
        /* Standard error stream handle */
        stdErr = GetStdHandle(STD_ERROR_HANDLE);

    DWORD inFlags;

    /* Some flags set by the system must be mainteined */
    GetConsoleMode(stdIn, &inFlags);

    if (!PI_HasFlag(inFlags, ENABLE_PROCESSED_INPUT))
    {
        inFlags |= ENABLE_PROCESSED_INPUT;

        /* Forces Ctrl+C system processing */
        SetConsoleMode(stdIn, inFlags);
    }

    DWORD outFlags;

    /* Some flags set by the system must be mainteined */
    GetConsoleMode(stdOut, &outFlags);

    if (!PI_HasFlag(outFlags, ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        outFlags |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        /* Enables colors and fonts processing in Win32 cmd output and error streams */
        SetConsoleMode(stdOut, outFlags);
        SetConsoleMode(stdErr, outFlags);
    }
#endif

    return;
}

void piSetConsoleTitle(const char *const title)
{
    if (PI_DEBUG && !pi_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    if (!SetConsoleTitleA((LPCSTR)title))
        piRaiseError("something went wrong", NULL);
#else
    printf("\033]0;%s\007", title);
#endif

    return;
}

void piSetCurrentDirectory(const char *const path)
{
    if (PI_DEBUG && !pi_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    if (!SetCurrentDirectoryA((LPCSTR)path))
        piRaiseError("something went wrong", NULL);
#else
    chdir(path);
#endif

    const size_t count = strlen(path);

    /* Copies the just set path in currentDir register */
    strncpy(pi_SystemInfo.currentDir, path, count);
    /* Assures the path ends with <NUL> */
    pi_SystemInfo.currentDir[count] = '\0';

    return;
}

const char *piGetCurrentDirectory(void)
{
    if (PI_DEBUG && !pi_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

    return pi_SystemInfo.currentDir;
}

size_t pi_GetPageSize(void)
{
    return pi_SystemInfo.pageSize;
}

/**
 * +---- Internal Errors Handling ---------+
 */

/**
 * @brief   Prints internal error trace (function, file and line).
 */
static inline void pi_PrintErrorTrace(const char *const func, const char *const file, const int line)
{
    eprintf("\n  ");

    if (func)
        eprintf("at " PI_BOLD_YELLOW "%s" PI_RESET "() ", func);
    
    if (file)
        eprintf("in " PI_WHITE "%s:%d" PI_RESET, file, line);
    else
        eprintf("in unspecified file and line" PI_RESET);

    eprintf("\n");

    return;
}

/**
 * Prints an error message with a given title.
 */
static inline void pi_ErrorAtV(const char *const func, const char *const file, const int line, const char *const title, const char *const format, va_list argList)
{
    if (title)
        eprintf("%s: ", title);

    veprintf(format, argList);
    pi_PrintErrorTrace(func, file, line);

    return;
}

void piWarningAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;
    
    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_WARNING, format, argList);
    va_end(argList);

    return;
}

void piErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;
    
    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_ERROR, format, argList);
    va_end(argList);

    return;
}

PI_NORET void piFatalErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    va_list argList;
    
    va_start(argList, format);
    pi_ErrorAtV(func, file, line, PI_TITLE_FATAL_ERROR, format, argList);
    va_end(argList);

    abort();

    /* No return */
}

/**
 * @brief   
 */
typedef struct _pi_ErrorHandler
{
    jmp_buf        *buf;
} PiErrorHandler;

static PiErrorHandler errorHandler;

void pi_SetErrorHandler(jmp_buf *const buf)
{
    assert(buf != NULL);

    /* Sets buf */
    errorHandler.buf = buf;

    return;
}

PI_NORET void piRaiseErrorAt(const char *const func, const char *const file, const int line, const char *const format, ...)
{
    const char *title;

    /* If is set an error handler the error could be recovered */
    if (errorHandler.buf)
        title = PI_BOLD PI_RED "error" PI_RESET;
    else
        title = PI_BOLD PI_DARK_RED "fatal error" PI_RESET;

    va_list argList;
    
    va_start(argList, format);
    pi_ErrorAtV(func, file, line, title, format, argList);
    va_end(argList);

    if (errorHandler.buf)
        /* Jump to error handler */
        longjmp(*(errorHandler.buf), !errno ? EXIT_FAILURE : errno);
    else
        abort();

    /* No return */
}

/**
 * +---- Internal Memory Management -------+
 */

/**
 * @brief   Checks if a memory allocation was successful.
 */
static inline const void *pi_CheckNotNull(const void *const block, const char *const func, const char *const file, const int line, const char *const message)
{
    if (!block)
        piFatalErrorAt(
            func,
            file,
            line,
            "%s",
            message ? message : "memory allocation failed"
        );

    return block;
}

#ifndef PI_XALLOC_ERROR_MESSAGE
#   define PI_XALLOC_ERROR_MESSAGE "cannot allocate enough memory"
#endif

void *pi_malloc(const size_t size)
{
    return (void *)pi_CheckNotNull(malloc(size), __func__, __FILE__, __LINE__, PI_XALLOC_ERROR_MESSAGE);
}

void *pi_calloc(const size_t count, const size_t size)
{
    return (void *)pi_CheckNotNull(calloc(count, size), __func__, __FILE__, __LINE__, PI_XALLOC_ERROR_MESSAGE);
}

#ifndef PI_REALLOC_ERROR_MESSAGE
#   define PI_REALLOC_ERROR_MESSAGE "cannot reallocate memory"
#endif

void *pi_realloc(void *const block, const size_t newSize)
{
    return (void *)pi_CheckNotNull(realloc(block, newSize), __func__, __FILE__, __LINE__, PI_REALLOC_ERROR_MESSAGE);
}

void *pi_resize(void *const block, const size_t oldSize, const size_t newSize)
{
    void *result;

    if (oldSize > 0)
    {
        if (newSize > 0)
            result = pi_realloc(block, newSize);
        else
            result = (free(block), NULL);
    }
    else
    {
        result = pi_malloc(newSize);
    }

    return result;
}

/* =------------------------------------------------------------= */
