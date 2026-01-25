#include "pi/runtime/env.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* =---- Platform Detection ----------------------------------------= */

#ifdef _WIN32
#   define PI_PLATFORM_WINDOWS 1
#   include <Windows.h>
#   include <io.h>
#   include <direct.h>
#else
#   define PI_PLATFORM_POSIX 1
#   include <unistd.h>
#   include <sys/stat.h>
#   include <errno.h>
#endif

/* =---- Stream Structure ------------------------------------------= */

/**
 * @brief   Internal stream structure.
 *
 * Wraps platform-specific file handles with additional metadata.
 */
struct _pi_Stream
{
    /**
     * @brief   Platform-specific file handle.
     *
     * On Windows: HANDLE from CreateFile()
     * On POSIX: FILE* from fopen()
     */
    void               *handle;
    /**
     * @brief   Stream flags used when opening.
     */
    PiStreamFlags       flags;
    /**
     * @brief   Marks standard streams (should not be closed).
     */
    bool                isStd;
};

/* =---- Platform-Specific File Operations -------------------------= */

#if PI_PLATFORM_WINDOWS

/**
 * @brief   Convert PiStreamFlags to Windows access mode.
 */
static DWORD pi_GetWindowsAccessMode(const PiStreamFlags flags)
{
    DWORD access = 0;

    if (flags & PI_STREAM_READ)
        access |= GENERIC_READ;

    if (flags & PI_STREAM_WRITE)
        access |= GENERIC_WRITE;

    return access;
}

/**
 * @brief   Convert PiStreamFlags to Windows creation disposition.
 */
static DWORD pi_GetWindowsCreationDisposition(const PiStreamFlags flags)
{
    if (flags & PI_STREAM_CREATE)
    {
        if (flags & PI_STREAM_TRUNCATE)
            return CREATE_ALWAYS;

        return OPEN_ALWAYS;
    }

    if (flags & PI_STREAM_TRUNCATE)
        return TRUNCATE_EXISTING;

    return OPEN_EXISTING;
}

/**
 * @brief   Convert Windows error code to PiIoResult.
 */
static PiIoResult pi_WindowsErrorToIoResult(const DWORD error)
{
    switch (error)
    {
    case ERROR_SUCCESS:
        return PI_IO_OK;
    case ERROR_FILE_NOT_FOUND:
    case ERROR_PATH_NOT_FOUND:
        return PI_IO_NOT_FOUND;
    case ERROR_ACCESS_DENIED:
        return PI_IO_ACCESS_DENIED;
    case ERROR_ALREADY_EXISTS:
    case ERROR_FILE_EXISTS:
        return PI_IO_EXISTS;
    case ERROR_DISK_FULL:
        return PI_IO_NO_SPACE;
    case ERROR_TOO_MANY_OPEN_FILES:
        return PI_IO_TOO_MANY_OPEN;
    case ERROR_INVALID_PARAMETER:
    case ERROR_INVALID_HANDLE:
        return PI_IO_INVALID;
    default:
        return PI_IO_ERROR;
    }
}

/**
 * @brief   Windows file open implementation.
 */
static PiStream *pi_WindowsFileOpen(PiEnv *const env, const char *const path, const PiStreamFlags flags)
{
    const DWORD access = pi_GetWindowsAccessMode(flags);
    const DWORD shareMode = FILE_SHARE_READ;
    const DWORD creation = pi_GetWindowsCreationDisposition(flags);
    const DWORD attributes = FILE_ATTRIBUTE_NORMAL;

    const HANDLE handle = CreateFileA(
        path,
        access,
        shareMode,
        NULL,
        creation,
        attributes,
        NULL
    );

    if (handle == INVALID_HANDLE_VALUE)
        return NULL;

    /* Handle append mode */
    if (flags & PI_STREAM_APPEND)
        SetFilePointer(handle, 0, NULL, FILE_END);

    /* Allocate stream structure */
    PiStream *const stream = (PiStream *)piEnvAlloc(env, sizeof(PiStream));

    if (!stream)
    {
        CloseHandle(handle);
        return NULL;
    }

    stream->handle = handle;
    stream->flags = flags;
    stream->isStd = false;

    return stream;
}

/**
 * @brief   Windows file close implementation.
 */
static PiIoResult pi_WindowsFileClose(PiEnv *const env, PiStream *const stream)
{
    if (!stream)
        return PI_IO_INVALID;

    if (stream->isStd)
        return PI_IO_OK; /* Don't close standard streams */

    if (stream->handle)
        CloseHandle((HANDLE)stream->handle);

    piEnvFree(env, stream);

    return PI_IO_OK;
}

/**
 * @brief   Windows file read implementation.
 */
static int64_t pi_WindowsFileRead(PiEnv *const env, PiStream *const stream, void *const buf, const size_t size)
{
    (void)env;

    if (!stream || !stream->handle || !buf)
        return PI_IO_INVALID;

    DWORD bytesRead = 0;

    if (!ReadFile((HANDLE)stream->handle, buf, (DWORD)size, &bytesRead, NULL))
        return pi_WindowsErrorToIoResult(GetLastError());

    if (bytesRead == 0)
        return PI_IO_EOF;

    return (int64_t)bytesRead;
}

/**
 * @brief   Windows file write implementation.
 */
static int64_t pi_WindowsFileWrite(PiEnv *const env, PiStream *const stream, const void *const data, const size_t size)
{
    (void)env;

    if (!stream || !stream->handle || !data)
        return PI_IO_INVALID;

    DWORD bytesWritten = 0;

    if (!WriteFile((HANDLE)stream->handle, data, (DWORD)size, &bytesWritten, NULL))
        return pi_WindowsErrorToIoResult(GetLastError());

    return (int64_t)bytesWritten;
}

/**
 * @brief   Windows file seek implementation.
 */
static int64_t pi_WindowsFileSeek(PiEnv *const env, PiStream *const stream, const int64_t offset, const PiSeekOrigin origin)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    DWORD moveMethod;

    switch (origin)
    {
    case PI_SEEK_SET:
        moveMethod = FILE_BEGIN;
        break;
    case PI_SEEK_CUR:
        moveMethod = FILE_CURRENT;
        break;
    case PI_SEEK_END:
        moveMethod = FILE_END;
        break;
    default:
        return PI_IO_INVALID;
    }

    LARGE_INTEGER li;

    li.QuadPart = offset;

    li.LowPart = SetFilePointer((HANDLE)stream->handle, li.LowPart, &li.HighPart, moveMethod);

    if (li.LowPart == INVALID_SET_FILE_POINTER && GetLastError() != NO_ERROR)
        return pi_WindowsErrorToIoResult(GetLastError());

    return li.QuadPart;
}

/**
 * @brief   Windows file tell implementation.
 */
static int64_t pi_WindowsFileTell(PiEnv *const env, PiStream *const stream)
{
    return pi_WindowsFileSeek(env, stream, 0, PI_SEEK_CUR);
}

/**
 * @brief   Windows file flush implementation.
 */
static PiIoResult pi_WindowsFileFlush(PiEnv *const env, PiStream *const stream)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    if (!FlushFileBuffers((HANDLE)stream->handle))
        return pi_WindowsErrorToIoResult(GetLastError());

    return PI_IO_OK;
}

/**
 * @brief   Windows file size implementation.
 */
static int64_t pi_WindowsFileSize(PiEnv *const env, PiStream *const stream)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    LARGE_INTEGER size;

    if (!GetFileSizeEx((HANDLE)stream->handle, &size))
        return pi_WindowsErrorToIoResult(GetLastError());

    return size.QuadPart;
}

/**
 * @brief   Windows file EOF check implementation.
 */
static bool pi_WindowsFileEof(PiEnv *const env, PiStream *const stream)
{
    const int64_t pos = pi_WindowsFileTell(env, stream);
    const int64_t size = pi_WindowsFileSize(env, stream);

    if (pos < 0 || size < 0)
        return true;

    return pos >= size;
}

/**
 * @brief   Windows file exists check implementation.
 */
static bool pi_WindowsFsExists(PiEnv *const env, const char *const path)
{
    (void)env;

    const DWORD attrs = GetFileAttributesA(path);

    return attrs != INVALID_FILE_ATTRIBUTES;
}

/**
 * @brief   Windows file delete implementation.
 */
static PiIoResult pi_WindowsFsDelete(PiEnv *const env, const char *const path)
{
    (void)env;

    if (!DeleteFileA(path))
        return pi_WindowsErrorToIoResult(GetLastError());

    return PI_IO_OK;
}

/**
 * @brief   Windows file rename implementation.
 */
static PiIoResult pi_WindowsFsRename(PiEnv *const env, const char *const oldPath, const char *const newPath)
{
    (void)env;

    if (!MoveFileA(oldPath, newPath))
        return pi_WindowsErrorToIoResult(GetLastError());

    return PI_IO_OK;
}

/**
 * @brief   Windows directory creation implementation.
 */
static PiIoResult pi_WindowsFsMkdir(PiEnv *const env, const char *const path)
{
    (void)env;

    if (!CreateDirectoryA(path, NULL))
        return pi_WindowsErrorToIoResult(GetLastError());

    return PI_IO_OK;
}

/**
 * @brief   Windows directory removal implementation.
 */
static PiIoResult pi_WindowsFsRmdir(PiEnv *const env, const char *const path)
{
    (void)env;

    if (!RemoveDirectoryA(path))
        return pi_WindowsErrorToIoResult(GetLastError());

    return PI_IO_OK;
}

/**
 * @brief   Windows get current directory implementation.
 */
static const char *pi_WindowsFsGetCwd(PiEnv *const env)
{
    return piEnvGetWorkingDir(env);
}

/**
 * @brief   Windows set current directory implementation.
 */
static PiIoResult pi_WindowsFsSetCwd(PiEnv *const env, const char *const path)
{
    return piEnvSetWorkingDir(env, path);
}

/**
 * @brief   Windows is directory check implementation.
 */
static bool pi_WindowsFsIsDir(PiEnv *const env, const char *const path)
{
    (void)env;

    const DWORD attrs = GetFileAttributesA(path);

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return (attrs & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

/**
 * @brief   Windows is file check implementation.
 */
static bool pi_WindowsFsIsFile(PiEnv *const env, const char *const path)
{
    (void)env;

    const DWORD attrs = GetFileAttributesA(path);

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

/**
 * @brief   Windows console read implementation.
 */
static int64_t pi_WindowsConsoleRead(PiEnv *const env, void *const buf, const size_t size)
{
    (void)env;

    const HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdIn == INVALID_HANDLE_VALUE)
        return PI_IO_ERROR;

    DWORD bytesRead = 0;

    if (!ReadFile(hStdIn, buf, (DWORD)size, &bytesRead, NULL))
        return pi_WindowsErrorToIoResult(GetLastError());

    return (int64_t)bytesRead;
}

/**
 * @brief   Windows console write implementation.
 */
static int64_t pi_WindowsConsoleWrite(PiEnv *const env, const int fd, const void *const data, const size_t size)
{
    (void)env;

    const HANDLE hStd = (fd == 2)
        ? GetStdHandle(STD_ERROR_HANDLE)
        : GetStdHandle(STD_OUTPUT_HANDLE);

    if (hStd == INVALID_HANDLE_VALUE)
        return PI_IO_ERROR;

    DWORD bytesWritten = 0;

    if (!WriteFile(hStd, data, (DWORD)size, &bytesWritten, NULL))
        return pi_WindowsErrorToIoResult(GetLastError());

    return (int64_t)bytesWritten;
}

#else /* PI_PLATFORM_POSIX */

/**
 * @brief   Convert PiStreamFlags to fopen mode string.
 */
static const char *pi_GetPosixOpenMode(const PiStreamFlags flags)
{
    const bool read = (flags & PI_STREAM_READ) != 0;
    const bool write = (flags & PI_STREAM_WRITE) != 0;
    const bool append = (flags & PI_STREAM_APPEND) != 0;
    const bool binary = (flags & PI_STREAM_BINARY) != 0;

    if (append)
        return binary ? (read ? "a+b" : "ab") : (read ? "a+" : "a");

    if (read && write)
        return binary ? "r+b" : "r+";

    if (write)
        return binary ? "wb" : "w";

    return binary ? "rb" : "r";
}

/**
 * @brief   Convert errno to PiIoResult.
 */
static PiIoResult pi_ErrnoToIoResult(const int error)
{
    switch (error)
    {
    case 0:
        return PI_IO_OK;
    case ENOENT:
        return PI_IO_NOT_FOUND;
    case EACCES:
    case EPERM:
        return PI_IO_ACCESS_DENIED;
    case EEXIST:
        return PI_IO_EXISTS;
    case ENOSPC:
        return PI_IO_NO_SPACE;
    case EMFILE:
    case ENFILE:
        return PI_IO_TOO_MANY_OPEN;
    case EINVAL:
        return PI_IO_INVALID;
    case EAGAIN:
#if EAGAIN != EWOULDBLOCK
    case EWOULDBLOCK:
#endif
        return PI_IO_WOULD_BLOCK;
    default:
        return PI_IO_ERROR;
    }
}

/**
 * @brief   POSIX file open implementation.
 */
static PiStream *pi_PosixFileOpen(PiEnv *const env, const char *const path, const PiStreamFlags flags)
{
    const char *const mode = pi_GetPosixOpenMode(flags);

    FILE *const fp = fopen(path, mode);

    if (!fp)
        return NULL;

    /* Allocate stream structure */
    PiStream *const stream = (PiStream *)piEnvAlloc(env, sizeof(PiStream));

    if (!stream)
    {
        fclose(fp);
        return NULL;
    }

    stream->handle = fp;
    stream->flags = flags;
    stream->isStd = false;

    return stream;
}

/**
 * @brief   POSIX file close implementation.
 */
static PiIoResult pi_PosixFileClose(PiEnv *const env, PiStream *const stream)
{
    if (!stream)
        return PI_IO_INVALID;

    if (stream->isStd)
        return PI_IO_OK; /* Don't close standard streams */

    if (stream->handle)
        fclose((FILE *)stream->handle);

    piEnvFree(env, stream);

    return PI_IO_OK;
}

/**
 * @brief   POSIX file read implementation.
 */
static int64_t pi_PosixFileRead(PiEnv *const env, PiStream *const stream, void *const buf, const size_t size)
{
    (void)env;

    if (!stream || !stream->handle || !buf)
        return PI_IO_INVALID;

    const size_t bytesRead = fread(buf, 1, size, (FILE *)stream->handle);

    if (bytesRead == 0)
    {
        if (feof((FILE *)stream->handle))
            return PI_IO_EOF;

        if (ferror((FILE *)stream->handle))
            return PI_IO_ERROR;
    }

    return (int64_t)bytesRead;
}

/**
 * @brief   POSIX file write implementation.
 */
static int64_t pi_PosixFileWrite(PiEnv *const env, PiStream *const stream, const void *const data, const size_t size)
{
    (void)env;

    if (!stream || !stream->handle || !data)
        return PI_IO_INVALID;

    const size_t bytesWritten = fwrite(data, 1, size, (FILE *)stream->handle);

    if (bytesWritten < size && ferror((FILE *)stream->handle))
        return PI_IO_ERROR;

    return (int64_t)bytesWritten;
}

/**
 * @brief   POSIX file seek implementation.
 */
static int64_t pi_PosixFileSeek(PiEnv *const env, PiStream *const stream, const int64_t offset, const PiSeekOrigin origin)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    int whence;

    switch (origin)
    {
    case PI_SEEK_SET:
        whence = SEEK_SET;
        break;
    case PI_SEEK_CUR:
        whence = SEEK_CUR;
        break;
    case PI_SEEK_END:
        whence = SEEK_END;
        break;
    default:
        return PI_IO_INVALID;
    }

    if (fseek((FILE *)stream->handle, (long)offset, whence) != 0)
        return pi_ErrnoToIoResult(errno);

    return ftell((FILE *)stream->handle);
}

/**
 * @brief   POSIX file tell implementation.
 */
static int64_t pi_PosixFileTell(PiEnv *const env, PiStream *const stream)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    const long pos = ftell((FILE *)stream->handle);

    if (pos < 0)
        return pi_ErrnoToIoResult(errno);

    return (int64_t)pos;
}

/**
 * @brief   POSIX file flush implementation.
 */
static PiIoResult pi_PosixFileFlush(PiEnv *const env, PiStream *const stream)
{
    (void)env;

    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    if (fflush((FILE *)stream->handle) != 0)
        return pi_ErrnoToIoResult(errno);

    return PI_IO_OK;
}

/**
 * @brief   POSIX file size implementation.
 */
static int64_t pi_PosixFileSize(PiEnv *const env, PiStream *const stream)
{
    if (!stream || !stream->handle)
        return PI_IO_INVALID;

    const int64_t currentPos = pi_PosixFileTell(env, stream);

    if (currentPos < 0)
        return currentPos;

    if (fseek((FILE *)stream->handle, 0, SEEK_END) != 0)
        return pi_ErrnoToIoResult(errno);

    const long size = ftell((FILE *)stream->handle);

    /* Restore position */
    fseek((FILE *)stream->handle, (long)currentPos, SEEK_SET);

    if (size < 0)
        return pi_ErrnoToIoResult(errno);

    return (int64_t)size;
}

/**
 * @brief   POSIX file EOF check implementation.
 */
static bool pi_PosixFileEof(PiEnv *const env, PiStream *const stream)
{
    (void)env;

    if (!stream || !stream->handle)
        return true;

    return feof((FILE *)stream->handle) != 0;
}

/**
 * @brief   POSIX file exists check implementation.
 */
static bool pi_PosixFsExists(PiEnv *const env, const char *const path)
{
    (void)env;

    return access(path, F_OK) == 0;
}

/**
 * @brief   POSIX file delete implementation.
 */
static PiIoResult pi_PosixFsDelete(PiEnv *const env, const char *const path)
{
    (void)env;

    if (unlink(path) != 0)
        return pi_ErrnoToIoResult(errno);

    return PI_IO_OK;
}

/**
 * @brief   POSIX file rename implementation.
 */
static PiIoResult pi_PosixFsRename(PiEnv *const env, const char *const oldPath, const char *const newPath)
{
    (void)env;

    if (rename(oldPath, newPath) != 0)
        return pi_ErrnoToIoResult(errno);

    return PI_IO_OK;
}

/**
 * @brief   POSIX directory creation implementation.
 */
static PiIoResult pi_PosixFsMkdir(PiEnv *const env, const char *const path)
{
    (void)env;

    if (mkdir(path, 0755) != 0)
        return pi_ErrnoToIoResult(errno);

    return PI_IO_OK;
}

/**
 * @brief   POSIX directory removal implementation.
 */
static PiIoResult pi_PosixFsRmdir(PiEnv *const env, const char *const path)
{
    (void)env;

    if (rmdir(path) != 0)
        return pi_ErrnoToIoResult(errno);

    return PI_IO_OK;
}

/**
 * @brief   POSIX get current directory implementation.
 */
static const char *pi_PosixFsGetCwd(PiEnv *const env)
{
    return piEnvGetWorkingDir(env);
}

/**
 * @brief   POSIX set current directory implementation.
 */
static PiIoResult pi_PosixFsSetCwd(PiEnv *const env, const char *const path)
{
    return piEnvSetWorkingDir(env, path);
}

/**
 * @brief   POSIX is directory check implementation.
 */
static bool pi_PosixFsIsDir(PiEnv *const env, const char *const path)
{
    (void)env;

    struct stat st;

    if (stat(path, &st) != 0)
        return false;

    return S_ISDIR(st.st_mode);
}

/**
 * @brief   POSIX is file check implementation.
 */
static bool pi_PosixFsIsFile(PiEnv *const env, const char *const path)
{
    (void)env;

    struct stat st;

    if (stat(path, &st) != 0)
        return false;

    return S_ISREG(st.st_mode);
}

/**
 * @brief   POSIX console read implementation.
 */
static int64_t pi_PosixConsoleRead(PiEnv *const env, void *const buf, const size_t size)
{
    (void)env;

    const ssize_t bytesRead = read(STDIN_FILENO, buf, size);

    if (bytesRead < 0)
        return pi_ErrnoToIoResult(errno);

    return (int64_t)bytesRead;
}

/**
 * @brief   POSIX console write implementation.
 */
static int64_t pi_PosixConsoleWrite(PiEnv *const env, const int fd, const void *const data, const size_t size)
{
    (void)env;

    const int fileno = (fd == 2) ? STDERR_FILENO : STDOUT_FILENO;
    const ssize_t bytesWritten = write(fileno, data, size);

    if (bytesWritten < 0)
        return pi_ErrnoToIoResult(errno);

    return (int64_t)bytesWritten;
}

#endif /* PI_PLATFORM_POSIX */

/* =---- Public API ------------------------------------------------= */

PI_Api(PiIoHandlers) piGetDefaultIoHandlers(void)
{
    PiIoHandlers handlers;

#if PI_PLATFORM_WINDOWS
    handlers.fileOpen       = pi_WindowsFileOpen;
    handlers.fileClose      = pi_WindowsFileClose;
    handlers.fileRead       = pi_WindowsFileRead;
    handlers.fileWrite      = pi_WindowsFileWrite;
    handlers.fileSeek       = pi_WindowsFileSeek;
    handlers.fileTell       = pi_WindowsFileTell;
    handlers.fileFlush      = pi_WindowsFileFlush;
    handlers.fileSize       = pi_WindowsFileSize;
    handlers.fileEof        = pi_WindowsFileEof;

    handlers.fsExists       = pi_WindowsFsExists;
    handlers.fsDelete       = pi_WindowsFsDelete;
    handlers.fsRename       = pi_WindowsFsRename;
    handlers.fsMkdir        = pi_WindowsFsMkdir;
    handlers.fsRmdir        = pi_WindowsFsRmdir;
    handlers.fsGetCwd       = pi_WindowsFsGetCwd;
    handlers.fsSetCwd       = pi_WindowsFsSetCwd;
    handlers.fsIsDir        = pi_WindowsFsIsDir;
    handlers.fsIsFile       = pi_WindowsFsIsFile;

    handlers.consoleRead    = pi_WindowsConsoleRead;
    handlers.consoleWrite   = pi_WindowsConsoleWrite;
#else
    handlers.fileOpen       = pi_PosixFileOpen;
    handlers.fileClose      = pi_PosixFileClose;
    handlers.fileRead       = pi_PosixFileRead;
    handlers.fileWrite      = pi_PosixFileWrite;
    handlers.fileSeek       = pi_PosixFileSeek;
    handlers.fileTell       = pi_PosixFileTell;
    handlers.fileFlush      = pi_PosixFileFlush;
    handlers.fileSize       = pi_PosixFileSize;
    handlers.fileEof        = pi_PosixFileEof;

    handlers.fsExists       = pi_PosixFsExists;
    handlers.fsDelete       = pi_PosixFsDelete;
    handlers.fsRename       = pi_PosixFsRename;
    handlers.fsMkdir        = pi_PosixFsMkdir;
    handlers.fsRmdir        = pi_PosixFsRmdir;
    handlers.fsGetCwd       = pi_PosixFsGetCwd;
    handlers.fsSetCwd       = pi_PosixFsSetCwd;
    handlers.fsIsDir        = pi_PosixFsIsDir;
    handlers.fsIsFile       = pi_PosixFsIsFile;

    handlers.consoleRead    = pi_PosixConsoleRead;
    handlers.consoleWrite   = pi_PosixConsoleWrite;
#endif

    return handlers;
}

PI_Api(PiIoHandlers) piGetNullIoHandlers(void)
{
    PiIoHandlers handlers;

    memset(&handlers, 0, sizeof(handlers));

    return handlers;
}

PI_Api(PiIoResult) piEnvSetWorkingDir(PiEnv *const env, const char *const path)
{
    if (!env)
        return PI_IO_INVALID;

    if (!path)
        return PI_IO_INVALID;

    const size_t len = strlen(path);
    const size_t requiredSize = len + 1;

    /* Reallocate buffer if needed */
    if (requiredSize > env->workingDirSize)
    {
        char *const newDir = (char *)piEnvRealloc(env, env->workingDir, requiredSize);

        if (!newDir)
            return PI_IO_ERROR;

        env->workingDir = newDir;
        env->workingDirSize = requiredSize;
    }

    memcpy(env->workingDir, path, requiredSize);

    return PI_IO_OK;
}

PI_Api(int64_t) piStreamReadLine(PiEnv *const env, PiStream *const stream, char *const buf, const size_t size)
{
    if (!env || !stream || !buf || size == 0)
        return PI_IO_INVALID;

    if (!env->io.fileRead)
        return PI_IO_NOT_SUPPORTED;

    size_t pos = 0;

    while (pos < size - 1)
    {
        char c;
        const int64_t result = env->io.fileRead(env, stream, &c, 1);

        if (result < 0)
        {
            if (pos > 0)
                break; /* Return what we have */

            return result;
        }

        if (result == 0)
            break; /* EOF */

        buf[pos++] = c;

        if (c == '\n')
            break;
    }

    buf[pos] = '\0';

    return (int64_t)pos;
}

PI_Api(int64_t) piStreamPrintf(PiEnv *const env, PiStream *const stream, const char *const format, ...)
{
    if (!env || !stream || !format)
        return PI_IO_INVALID;

    if (!env->io.fileWrite)
        return PI_IO_NOT_SUPPORTED;

    char buffer[PI_IO_BUFFER_SIZE];
    va_list args;

    va_start(args, format);
    const int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0)
        return PI_IO_ERROR;

    const size_t writeSize = (size_t)len < sizeof(buffer) ? (size_t)len : sizeof(buffer) - 1;

    return env->io.fileWrite(env, stream, buffer, writeSize);
}

PI_Api(char *) piReadEntireFile(PiEnv *const env, const char *const path, size_t *const outSize)
{
    if (!env || !path)
        return NULL;

    if (!env->io.fileOpen || !env->io.fileClose || !env->io.fileRead || !env->io.fileSize)
        return NULL;

    PiStream *const stream = env->io.fileOpen(env, path, PI_STREAM_READ | PI_STREAM_BINARY);

    if (!stream)
        return NULL;

    const int64_t size = env->io.fileSize(env, stream);

    if (size < 0)
    {
        env->io.fileClose(env, stream);
        return NULL;
    }

    char *const buffer = (char *)piEnvAlloc(env, (size_t)size + 1);

    if (!buffer)
    {
        env->io.fileClose(env, stream);
        return NULL;
    }

    const int64_t bytesRead = env->io.fileRead(env, stream, buffer, (size_t)size);

    env->io.fileClose(env, stream);

    if (bytesRead < 0)
    {
        piEnvFree(env, buffer);
        return NULL;
    }

    buffer[bytesRead] = '\0';

    if (outSize)
        *outSize = (size_t)bytesRead;

    return buffer;
}

PI_Api(PiIoResult) piWriteEntireFile(PiEnv *const env, const char *const path, const void *const data, const size_t size)
{
    if (!env || !path || !data)
        return PI_IO_INVALID;

    if (!env->io.fileOpen || !env->io.fileClose || !env->io.fileWrite)
        return PI_IO_NOT_SUPPORTED;

    PiStream *const stream = env->io.fileOpen(env, path, PI_STREAM_WRITE | PI_STREAM_CREATE | PI_STREAM_TRUNCATE | PI_STREAM_BINARY);

    if (!stream)
        return PI_IO_ERROR;

    const int64_t bytesWritten = env->io.fileWrite(env, stream, data, size);

    env->io.fileClose(env, stream);

    if (bytesWritten < 0)
        return (PiIoResult)bytesWritten;

    if ((size_t)bytesWritten != size)
        return PI_IO_ERROR;

    return PI_IO_OK;
}

/* =------------------------------------------------------------= */
