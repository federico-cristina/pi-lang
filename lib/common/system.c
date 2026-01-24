#include "pi/common/system.h"
#include "pi/common/error.h"

#include <string.h>
#include <errno.h>

#ifdef _WIN32
#   include <Windows.h>
#else
#   include <unistd.h>
#endif

/* =---- System State ------------------------------------------= */

/**
 * @brief   System information record.
 */
typedef struct _pi_SystemInfo
{
    /**
     * @brief   Initialization flag.
     */
    bool           isInit;
    /**
     * @brief   Memory page size.
     */
    size_t         pageSize;
    /**
     * @brief   CPU architecture name.
     */
    const char    *cpuName;
    /**
     * @brief   Operating system name.
     */
    const char    *osName;
    /**
     * @brief   Current working directory (dynamically allocated).
     */
    char          *currentDir;
    /**
     * @brief   Size of current directory buffer.
     */
    size_t         currentDirSize;
} PiSystemInfo;

/**
 * @brief   Global system state.
 */
static PiSystemInfo g_SystemInfo = { 0 };

/* =---- Platform-Specific Helpers -----------------------------= */

#ifdef _WIN32
/**
 * @brief   Gets CPU architecture name from Windows processor architecture ID.
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
        result = "unknown";
        break;
    }

    return result;
}
#endif

/* =---- System Initialization ---------------------------------= */

PI_Api(void) piInitSystem(void)
{
    if (g_SystemInfo.isInit)
        return;

    g_SystemInfo.isInit = true;

#ifdef _WIN32
    /* Determine OS name */
#   ifdef _WIN64
    g_SystemInfo.osName = "Win64";
#   else
    g_systemInfo.osName = "Win32";
#   endif

    SYSTEM_INFO sysInfo;

    /* Retrives system info */
    GetSystemInfo(&sysInfo);

    /* Store system page size */
    g_SystemInfo.pageSize = (size_t)sysInfo.dwPageSize;
    /* Store processor architecture */
    g_SystemInfo.cpuName = pi_GetArchitectureName(sysInfo.wProcessorArchitecture);

    /* Dynamically allocate current directory buffer (improved from MAX_PATH) */
    const DWORD requiredSize = GetCurrentDirectoryA(0, NULL);

    if (requiredSize > 0)
    {
        /* Allocate exact size needed (not limited to MAX_PATH) */
        char *currentDir = (char *)calloc(requiredSize, sizeof(char));

        if (!currentDir)
            piFatal("cannot allocate memory for current directory", NULL);

        GetCurrentDirectoryA(requiredSize, currentDir);

        g_SystemInfo.currentDir = currentDir;
        g_SystemInfo.currentDirSize = requiredSize;
    }
    else
    {
        /* Failed to determine current directory */
        g_SystemInfo.currentDir = NULL;
        g_SystemInfo.currentDirSize = 0;
    }
#else
    /* Unix-like systems */
    g_SystemInfo.pageSize = sysconf(_SC_PAGESIZE);

    /* Get current directory with dynamic sizing */
    char *const cwd = getcwd(NULL, 0);  /* glibc extension: allocates buffer */

    if (cwd)
    {
        g_SystemInfo.currentDir = cwd;
        g_SystemInfo.currentDirSize = strlen(cwd) + 1;
    }
    else
    {
        g_SystemInfo.currentDir = NULL;
        g_SystemInfo.currentDirSize = 0;
    }
#endif

    /* Enable colors if requested */
#if PI_USE_COLORS
    piEnableVirtualTerminal();
#endif

    return;
}

PI_Api(void) piFreeSystem(void)
{
    if (!g_SystemInfo.isInit)
        return;

    /* Free current directory buffer if allocated */
    if (g_SystemInfo.currentDir)
        free(g_SystemInfo.currentDir);

    /* Reset all fields to zero */
    memset(&g_SystemInfo, 0, sizeof(g_SystemInfo));

    return;
}

/* =---- Platform Utilities ------------------------------------= */

PI_Api(void) piEnableVirtualTerminal(void)
{
    if (PI_DEBUG && !g_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    const HANDLE
        stdIn = GetStdHandle(STD_INPUT_HANDLE),
        stdOut = GetStdHandle(STD_OUTPUT_HANDLE),
        stdErr = GetStdHandle(STD_ERROR_HANDLE);

    DWORD inFlags;

    GetConsoleMode(stdIn, &inFlags);

    if (!PI_HasFlag(inFlags, ENABLE_PROCESSED_INPUT))
        SetConsoleMode(stdIn, inFlags | ENABLE_PROCESSED_INPUT);

    DWORD outFlags;

    GetConsoleMode(stdOut, &outFlags);

    if (!PI_HasFlag(outFlags, ENABLE_VIRTUAL_TERMINAL_PROCESSING))
    {
        outFlags |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

        SetConsoleMode(stdOut, outFlags);
        SetConsoleMode(stdErr, outFlags);
    }
#endif

    return;
}

PI_Api(void) piSetConsoleTitle(const char *const title)
{
    if (PI_DEBUG && !g_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    if (!SetConsoleTitleA((LPCSTR)title))
        piRaiseError("failed to set console title", NULL);
#else
    printf("\033]0;%s\007", title);
#endif

    return;
}

PI_Api(void) piSetCurrentDirectory(const char *const path)
{
    if (PI_DEBUG && !g_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

#ifdef _WIN32
    if (!SetCurrentDirectoryA((LPCSTR)path))
        piRaiseError("failed to change directory", NULL);

    /* Reallocate buffer if needed */
    const DWORD requiredSize = GetCurrentDirectoryA(0, NULL);

    if (requiredSize > g_SystemInfo.currentDirSize)
    {
        char *const newDir = (char *)realloc(g_SystemInfo.currentDir, requiredSize);

        if (!newDir)
            piFatal("cannot reallocate directory buffer", NULL);

        g_SystemInfo.currentDir = newDir;
        g_SystemInfo.currentDirSize = requiredSize;
    }

    GetCurrentDirectoryA(g_SystemInfo.currentDirSize, g_SystemInfo.currentDir);
#else
    if (chdir(path) != 0)
        piRaiseError("failed to change directory: %s", strerror(errno));

    /* Update cached path */
    free(g_systemInfo.currentDir);

    g_systemInfo.currentDir = getcwd(NULL, 0);

    if (g_systemInfo.currentDir)
    {
        g_systemInfo.currentDirSize = strlen(g_systemInfo.currentDir) + 1;
    }
    else
    {
        g_systemInfo.currentDirSize = 0;

        piRaiseError("failed to get current directory after chdir", NULL);
    }
#endif

    return;
}

PI_Api(const char *) piGetCurrentDirectory(void)
{
    if (PI_DEBUG && !g_SystemInfo.isInit)
        piFatal("system has not been initialized", NULL);

    return g_SystemInfo.currentDir;
}

PI_Api(size_t) piGetPageSize(void)
{
    return g_SystemInfo.pageSize;
}

/* =------------------------------------------------------------= */
