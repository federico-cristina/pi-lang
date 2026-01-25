#pragma once

/**
 * @file        io.h
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
 * @brief       Abstract I/O system for environment-isolated stream operations.
 *
 *              Provides a VTable-based I/O abstraction that allows environments to:
 *               - Use custom file and console I/O handlers
 *               - Implement sandboxed file system access
 *               - Redirect standard streams (stdin, stdout, stderr)
 *               - Support capability-based I/O restrictions
 */

#ifndef _PI_RUNTIME_IO_H
#define _PI_RUNTIME_IO_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Forward Declarations -------------------------------------= */

/**
 * @brief   Forward declaration of environment structure.
 */
typedef struct _pi_Environment PiEnv;

/* =---- Stream Types ----------------------------------------------= */

/**
 * @brief   Opaque stream handle.
 *
 * Represents a platform-specific stream (file, console, pipe, etc.).
 * The actual implementation is hidden to allow different backends.
 */
typedef struct _pi_Stream PiStream;

/**
 * @brief   Stream open flags.
 *
 * These flags control how a stream is opened. Multiple flags can be
 * combined using bitwise OR.
 */
typedef enum _pi_StreamFlags
{
    /**
     * @brief   Open for reading.
     */
    PI_STREAM_READ      = PI_BitFlag(1),
    /**
     * @brief   Open for writing.
     */
    PI_STREAM_WRITE     = PI_BitFlag(2),
    /**
     * @brief   Append to end of file.
     */
    PI_STREAM_APPEND    = PI_BitFlag(3),
    /**
     * @brief   Create file if it doesn't exist.
     */
    PI_STREAM_CREATE    = PI_BitFlag(4),
    /**
     * @brief   Truncate file to zero length.
     */
    PI_STREAM_TRUNCATE  = PI_BitFlag(5),
    /**
     * @brief   Open in binary mode (no text translation).
     */
    PI_STREAM_BINARY    = PI_BitFlag(6),
    /**
     * @brief   Non-blocking I/O (if supported).
     */
    PI_STREAM_NONBLOCK  = PI_BitFlag(7),
} PiStreamFlags;

/**
 * @brief   Seek origin constants.
 *
 * These constants specify the reference point for seek operations.
 */
typedef enum _pi_SeekOrigin
{
    /**
     * @brief   Seek from the beginning of the stream.
     */
    PI_SEEK_SET = 0,
    /**
     * @brief   Seek from the current position.
     */
    PI_SEEK_CUR = 1,
    /**
     * @brief   Seek from the end of the stream.
     */
    PI_SEEK_END = 2,
} PiSeekOrigin;

/**
 * @brief   I/O operation result codes.
 *
 * These codes indicate the result of I/O operations.
 * Non-negative values indicate success (often byte counts).
 * Negative values indicate errors.
 */
typedef enum _pi_IoResult
{
    /**
     * @brief   Operation completed successfully.
     */
    PI_IO_OK            =  0,
    /**
     * @brief   Generic I/O error.
     */
    PI_IO_ERROR         = -1,
    /**
     * @brief   End of file reached.
     */
    PI_IO_EOF           = -2,
    /**
     * @brief   Operation would block (non-blocking mode).
     */
    PI_IO_WOULD_BLOCK   = -3,
    /**
     * @brief   Access denied (permission error).
     */
    PI_IO_ACCESS_DENIED = -4,
    /**
     * @brief   File or path not found.
     */
    PI_IO_NOT_FOUND     = -5,
    /**
     * @brief   File or resource already exists.
     */
    PI_IO_EXISTS        = -6,
    /**
     * @brief   Invalid argument provided.
     */
    PI_IO_INVALID       = -7,
    /**
     * @brief   No space left on device.
     */
    PI_IO_NO_SPACE      = -8,
    /**
     * @brief   Too many open files.
     */
    PI_IO_TOO_MANY_OPEN = -9,
    /**
     * @brief   Operation not supported.
     */
    PI_IO_NOT_SUPPORTED = -10,
} PiIoResult;

/* =---- File I/O Handler Types ------------------------------------= */

/**
 * @brief   File open handler function type.
 *
 * @param[in] env    Environment context.
 * @param[in] path   Path to the file to open.
 * @param[in] flags  Open flags (PiStreamFlags).
 *
 * @return  Pointer to stream on success, NULL on failure.
 */
typedef PiStream *(*PiFileOpenFn)(PiEnv *const env, const char *const path, const PiStreamFlags flags);
/**
 * @brief   File close handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to close.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFileCloseFn)(PiEnv *const env, PiStream *const stream);

/**
 * @brief   File read handler function type.
 *
 * @param[in]  env     Environment context.
 * @param[in]  stream  Stream to read from.
 * @param[out] buf     Buffer to read into.
 * @param[in]  size    Maximum number of bytes to read.
 *
 * @return  Number of bytes read (>= 0), or negative error code.
 */
typedef int64_t (*PiFileReadFn)(PiEnv *const env, PiStream *const stream, void *const buf, const size_t size);
/**
 * @brief   File write handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to write to.
 * @param[in] data    Data to write.
 * @param[in] size    Number of bytes to write.
 *
 * @return  Number of bytes written (>= 0), or negative error code.
 */
typedef int64_t (*PiFileWriteFn)(PiEnv *const env, PiStream *const stream, const void *data, size_t size);

/**
 * @brief   File seek handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to seek in.
 * @param[in] offset  Offset in bytes.
 * @param[in] origin  Reference point (PiSeekOrigin).
 *
 * @return  New position (>= 0), or negative error code.
 */
typedef int64_t (*PiFileSeekFn)(PiEnv *const env, PiStream *const stream, const int64_t offset, const PiSeekOrigin origin);
/**
 * @brief   File tell handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to query.
 *
 * @return  Current position (>= 0), or negative error code.
 */
typedef int64_t (*PiFileTellFn)(PiEnv *const env, PiStream *const stream);
/**
 * @brief   File flush handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to flush.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFileFlushFn)(PiEnv *const env, PiStream *const stream);

/**
 * @brief   File size handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to query.
 *
 * @return  File size in bytes (>= 0), or negative error code.
 */
typedef int64_t (*PiFileSizeFn)(PiEnv *const env, PiStream *const stream);
/**
 * @brief   File EOF check handler function type.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to check.
 *
 * @return  true if at end of file, false otherwise.
 */
typedef bool (*PiFileEofFn)(PiEnv *const env, PiStream *const stream);

/* =---- File System Handler Types ---------------------------------= */

/**
 * @brief   File exists check handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path to check.
 *
 * @return  true if file exists, false otherwise.
 */
typedef bool (*PiFsExistsFn)(PiEnv *const env, const char *const path);

/**
 * @brief   File delete handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path to delete.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFsDeleteFn)(PiEnv *const env, const char *const path);
/**
 * @brief   File/directory rename handler function type.
 *
 * @param[in] env      Environment context.
 * @param[in] oldPath  Current path.
 * @param[in] newPath  New path.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFsRenameFn)(PiEnv *const env, const char *const oldPath, const char *const newPath);

/**
 * @brief   Directory creation handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path of directory to create.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFsMkdirFn)(PiEnv *const env, const char *const path);
/**
 * @brief   Directory removal handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path of directory to remove.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFsRmdirFn)(PiEnv *const env, const char *const path);

/**
 * @brief   Get current working directory handler function type.
 *
 * @param[in] env  Environment context.
 *
 * @return  Pointer to current directory string (env-owned), or NULL on failure.
 */
typedef const char *(*PiFsGetCwdFn)(PiEnv *const env);
/**
 * @brief   Set current working directory handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  New working directory path.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
typedef PiIoResult (*PiFsSetCwdFn)(PiEnv *const env, const char *const path);

/**
 * @brief   Check if path is a directory handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path to check.
 *
 * @return  true if path is a directory, false otherwise.
 */
typedef bool (*PiFsIsDirFn)(PiEnv *const env, const char *const path);
/**
 * @brief   Check if path is a regular file handler function type.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path to check.
 *
 * @return  true if path is a regular file, false otherwise.
 */
typedef bool (*PiFsIsFileFn)(PiEnv *const env, const char *const path);

/* =---- Console I/O Handler Types ---------------------------------= */

/**
 * @brief   Console read handler function type.
 *
 * Reads from standard input.
 *
 * @param[in]  env   Environment context.
 * @param[out] buf   Buffer to read into.
 * @param[in]  size  Maximum number of bytes to read.
 *
 * @return  Number of bytes read (>= 0), or negative error code.
 */
typedef int64_t (*PiConsoleReadFn)(PiEnv *const env, void *const buf, const size_t size);
/**
 * @brief   Console write handler function type.
 *
 * Writes to standard output or error.
 *
 * @param[in] env   Environment context.
 * @param[in] fd    File descriptor (1 = stdout, 2 = stderr).
 * @param[in] data  Data to write.
 * @param[in] size  Number of bytes to write.
 *
 * @return  Number of bytes written (>= 0), or negative error code.
 */
typedef int64_t (*PiConsoleWriteFn)(PiEnv *const env, const int fd, const void *const data, const size_t size);

/* =---- I/O Handlers VTable ---------------------------------------= */

/**
 * @brief   I/O handler function table.
 *
 * Contains function pointers for all I/O operations. These can be customized
 * to implement sandboxing, redirection, or mock I/O for testing.
 *
 * @note    Any handler can be NULL to indicate the operation is not supported.
 */
typedef struct _pi_IoHandlers
{
    /* ---- File Operations ---- */

    /**
     * @brief   Open a file stream.
     */
    PiFileOpenFn    fileOpen;
    /**
     * @brief   Close a file stream.
     */
    PiFileCloseFn   fileClose;
    /**
     * @brief   Read from a file stream.
     */
    PiFileReadFn    fileRead;
    /**
     * @brief   Write to a file stream.
     */
    PiFileWriteFn   fileWrite;
    /**
     * @brief   Seek within a file stream.
     */
    PiFileSeekFn    fileSeek;
    /**
     * @brief   Get current position in file stream.
     */
    PiFileTellFn    fileTell;
    /**
     * @brief   Flush file stream buffers.
     */
    PiFileFlushFn   fileFlush;
    /**
     * @brief   Get file size.
     */
    PiFileSizeFn    fileSize;
    /**
     * @brief   Check if at end of file.
     */
    PiFileEofFn     fileEof;

    /* ---- File System Operations ---- */

    /**
     * @brief   Check if path exists.
     */
    PiFsExistsFn    fsExists;
    /**
     * @brief   Delete a file.
     */
    PiFsDeleteFn    fsDelete;
    /**
     * @brief   Rename a file or directory.
     */
    PiFsRenameFn    fsRename;
    /**
     * @brief   Create a directory.
     */
    PiFsMkdirFn     fsMkdir;
    /**
     * @brief   Remove a directory.
     */
    PiFsRmdirFn     fsRmdir;
    /**
     * @brief   Get current working directory.
     */
    PiFsGetCwdFn    fsGetCwd;
    /**
     * @brief   Set current working directory.
     */
    PiFsSetCwdFn    fsSetCwd;
    /**
     * @brief   Check if path is a directory.
     */
    PiFsIsDirFn     fsIsDir;
    /**
     * @brief   Check if path is a regular file.
     */
    PiFsIsFileFn    fsIsFile;

    /* ---- Console Operations ---- */

    /**
     * @brief   Read from console (stdin).
     */
    PiConsoleReadFn     consoleRead;
    /**
     * @brief   Write to console (stdout/stderr).
     */
    PiConsoleWriteFn    consoleWrite;
} PiIoHandlers;

/* =---- Standard Streams ------------------------------------------= */

/**
 * @brief   Standard stream handles.
 *
 * Contains handles to the standard input, output, and error streams.
 * These are initialized when the environment is created and can be
 * redirected to files or other streams.
 */
typedef struct _pi_StdStreams
{
    /**
     * @brief   Standard input stream.
     */
    PiStream *in;
    /**
     * @brief   Standard output stream.
     */
    PiStream *out;
    /**
     * @brief   Standard error stream.
     */
    PiStream *err;
} PiStdStreams;

/* =---- I/O Configuration -----------------------------------------= */

#ifndef PI_IO_MAX_OPEN_FILES
/**
 * @brief   Maximum number of open files per environment.
 */
#   define PI_IO_MAX_OPEN_FILES 256
#endif

#ifndef PI_IO_BUFFER_SIZE
/**
 * @brief   Default I/O buffer size.
 */
#   define PI_IO_BUFFER_SIZE 4096
#endif

/* =---- I/O Handler Accessors -------------------------------------= */

/**
 * @brief   Get the default (platform-native) I/O handlers.
 *
 * Returns a handlers structure with platform-specific implementations
 * for all I/O operations. These handlers use the native file system.
 *
 * @return  Default I/O handlers.
 */
PI_Api(PiIoHandlers) piGetDefaultIoHandlers(void);
/**
 * @brief   Get null I/O handlers.
 *
 * Returns a handlers structure where all operations are no-ops or return
 * errors. Useful for sandboxed environments with no I/O access.
 *
 * @return  Null I/O handlers.
 */
PI_Api(PiIoHandlers) piGetNullIoHandlers(void);

/* =---- Stream Helper Functions -----------------------------------= */

/**
 * @brief   Read a line from a stream.
 *
 * Reads characters until a newline or EOF is reached, or the buffer is full.
 * The newline is included in the buffer if present.
 *
 * @param[in]  env     Environment context.
 * @param[in]  stream  Stream to read from.
 * @param[out] buf     Buffer to store the line.
 * @param[in]  size    Size of buffer.
 *
 * @return  Number of bytes read (including newline), or negative error code.
 */
PI_Api(int64_t) piStreamReadLine(PiEnv *const env, PiStream *const stream, char *const buf, const size_t size);
/**
 * @brief   Write a formatted string to a stream.
 *
 * @param[in] env     Environment context.
 * @param[in] stream  Stream to write to.
 * @param[in] format  Printf-style format string.
 * @param[in] ...     Format arguments.
 *
 * @return  Number of bytes written (>= 0), or negative error code.
 */
PI_Api(int64_t) piStreamPrintf(PiEnv *const env, PiStream *const stream, const char *const format, ...);

/**
 * @brief   Read all contents of a file into a buffer.
 *
 * Allocates a buffer using the environment's allocator and reads the
 * entire file contents into it.
 *
 * @param[in]  env      Environment context.
 * @param[in]  path     Path to file.
 * @param[out] outSize  Receives the size of the file.
 *
 * @return  Pointer to allocated buffer containing file contents, or NULL on failure.
 *          Caller is responsible for freeing with piEnvFree().
 */
PI_Api(char *) piReadEntireFile(PiEnv *const env, const char *const path, size_t *const outSize);
/**
 * @brief   Write a buffer to a file.
 *
 * Creates or truncates the file and writes the entire buffer.
 *
 * @param[in] env   Environment context.
 * @param[in] path  Path to file.
 * @param[in] data  Data to write.
 * @param[in] size  Size of data in bytes.
 *
 * @return  PI_IO_OK on success, error code on failure.
 */
PI_Api(PiIoResult) piWriteEntireFile(PiEnv *const env, const char *const path, const void *const data, const size_t size);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
