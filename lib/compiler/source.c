/**
 * @file        source.c
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
 * @brief       Source stream implementation with multi-source support.
 */

#include "pi/compiler/source.h"

#include <string.h>

/* =---- Source Streams ----------------------------------------= */

#ifndef PI_DEFAULT_SOURCE_FOPEN_MODE
#   define PI_DEFAULT_SOURCE_FOPEN_MODE "r"
#endif

/**
 * Gets the file name from a given path.
 */
static inline const char *pi_GetFileName(const char *const path)
{
    assert(path != NULL);

    const char *p = (const char *)path, *result = p;

    for (p = path; *p; p++)
    {
        if ((*p == '/') || (*p == '\\'))
            result = p + 1;
    }

    return result;
}

/**
 * Retrieves the size of a file stream.
 */
static inline size_t pi_GetFileSize(FILE *const stream)
{
    assert(stream != NULL);

    fpos_t pos;

    /* Saves current position */
    fgetpos(stream, &pos);
    /* Seeks to the end of the file */
#ifdef _WIN32
    _fseeki64(stream, 0, SEEK_END);
#else
    fseek(stream, 0, SEEK_END);
#endif
    /* Gets the size of the file */
#ifdef _WIN32
    const size_t size = (size_t)_ftelli64(stream);
#else
    const size_t size = (size_t)ftell(stream);
#endif
    /* Resets file position */
    fsetpos(stream, &pos);

    return size;
}

#ifndef pi_StringOrEmpty
/* This macro gets an empty string instead of NULL. */
#   define pi_StringOrEmpty(str) (!(str) ? "" : (str))
#endif

/**
 * +---- Source ---------------------------+
 */

static inline PiSource *pi_InitSourceBuffer(PiSource *const source, char *const buffer, const size_t size)
{
    /* If not specified a buffer, a new one is allocated */
    if (!buffer)
        source->buffer = PI_NewArray(char, size);
    else
        source->buffer = buffer;

    source->bufferSize = size;

    /* Initializes offset */
    source->delta = 0;
    /* Initializes encoding format */
    source->encoding = PI_SOURCE_ENCODING_ASCII;

    /* Resets positions */
    piResetSourcePosition(&source->currentPos);
    piResetSourcePosition(&source->forwardPos);

    return source;
}

static inline PiSource *pi_InitSourceStream(PiSource *const source, FILE *const stream)
{
    assert(stream != NULL);

    /* Sets stream info */
    source->stream = stream;
    source->streamSize = pi_GetFileSize(stream);

    /* Gets default system page size */
    const size_t pageSize = piGetPageSize();

    /* Initializes a new buffer */
    pi_InitSourceBuffer(source, NULL, source->streamSize > pageSize ? pageSize : source->streamSize);
    /* Reads a new portion of the file */
    fread((void *)source->buffer, sizeof(char), source->bufferSize, stream);

    return source;
}

PiSource *piOpenSourceBuffer(PiSource *const source, const char *const name, char *const buffer, const size_t size)
{
    assert(source != NULL);

    /* Sets naming info */
    source->name = pi_StringOrEmpty(name);
    source->path = NULL;
    
    pi_InitSourceBuffer(source, buffer, size);

    /* Initializes the stream info */
    source->stream = NULL;
    source->streamSize = 0;
    
    return source;
}

PiSource *piOpenSourceStream(PiSource *const source, const char *const name, FILE *const stream)
{
    assert(source != NULL);

    /* Sets naming info */
    source->name = pi_StringOrEmpty(name);
    source->path = NULL;
    
    /* Sets stream typology */
    source->kind = PI_SOURCE_KIND_FILE;

    /* Initializes the stream info */
    return pi_InitSourceStream(source, stream);
}

PiSource *piOpenSourceText(PiSource *const source, const char *const name, const char *const text)
{
    assert(source != NULL);

    /* Sets naming info */
    source->name = pi_StringOrEmpty(name);
    source->path = NULL;

    /* Sets stream typology */
    source->kind = PI_SOURCE_KIND_TEXT;

    return pi_InitSourceBuffer(source, (char *)text, strlen(text));
}

PiSource *piOpenSourceFile(PiSource *const source, const char *const path)
{
    assert(source != NULL);

    /* Sets naming info */
    source->name = pi_GetFileName(path);
    source->path = path;

    FILE *const stream = fopen(path, PI_DEFAULT_SOURCE_FOPEN_MODE);

    if (!stream)
        piRaiseError("failed to open source file '%s'", path);

    /* Sets stream typology */
    source->kind = PI_SOURCE_KIND_FILE;

    return pi_InitSourceStream(source, stream);
}

PiSource *piCloseSource(PiSource *const source)
{
    assert(source != NULL);

    if (source->stream)
    {
        /* Closes the file stream */
        fclose(source->stream);
        /* Frees the allocated buffer */
        free(source->buffer);
    }

    /* Resets naming info */
    source->name = NULL;
    source->path = NULL;

    /* Resets stream info */
    source->stream = NULL;
    source->streamSize = 0;

    /* Resets buffer info */
    source->buffer = NULL;
    source->bufferSize = 0;

    /* Resets delta counter */
    source->delta = 0;

    /* Resets kind and encoding */
    source->kind = -1;
    source->encoding = -1;
    
    /* Resets current position marker */
    source->currentPos.offset = 0;
    source->currentPos.line = 0;
    source->currentPos.column = 0;

    /* Resets forward position marker */
    source->forwardPos.offset = 0;
    source->forwardPos.line = 0;
    source->forwardPos.column = 0;

    return source;
}

static inline bool pi_SourceIsOpen(const PiSource *const source)
{
    /* 'name' field is always set when source is open */
    return !!source->name;
}

static inline bool pi_IsSourceFile(const PiSource *const source)
{
    return source->kind == PI_SOURCE_KIND_FILE;
}

static inline int pi_SourcePeek(const PiSource *const source)
{
    int result;

    if (source->forwardPos.offset < source->bufferSize)
    {
        result = source->buffer[source->forwardPos.offset];

        if (result == '\0')
            result = EOF;
    }
    else
    {
        result = EOF;
    }

    return result;
}

int piSourcePeek(const PiSource *const source)
{
    assert(source != NULL);

    if (!pi_SourceIsOpen(source))
        piRaiseError("tried to peek from a closed source stream", NULL);

    return pi_SourcePeek(source);
}

static inline size_t pi_GetSourcePositionDelta(const PiSourcePos *const startPos, const PiSourcePos *const endPos)
{
    return (size_t)(endPos->offset - startPos->offset);
}

static int pi_SourceRead(PiSource *const source)
{
    int result = source->buffer[source->forwardPos.offset];

    if (result != '\0')
    {
        if (result == '\n')
            ++source->forwardPos.line, source->forwardPos.column = 1;
        else
            ++source->forwardPos.column;

        ++source->forwardPos.offset;
    }
    else
    {
        result = -1;
    }

    return result;
}

static inline int pi_SourceReadFile(PiSource *const source)
{
    int result;

    if (source->delta < source->streamSize)
    {
        const size_t posDelta = pi_GetSourcePositionDelta(&source->currentPos, &source->forwardPos);

        /* Moves the lexeme that is currently under the scanner at the beginning of the buffer */
        memcpy((void *)source->buffer, (const void *)&source->buffer[source->currentPos.offset], posDelta);
        /* Reads a new portion of the file */
        fread((void *)&source->buffer[posDelta], sizeof(char), source->bufferSize - posDelta, source->stream);

        /* Increments delta from the beginning of the stream */
        source->delta += source->currentPos.offset;
        /* Sets the current position to the beginning of the buffer */
        source->currentPos.offset = 0;
        /* Sets the forward position to the delta */
        source->forwardPos.offset = posDelta;

        /* Reads the character */
        result = pi_SourceRead(source);
    }
    else
    {
        /* End of the stream reached */
        result = EOF;
    }

    return result;
}

static inline size_t pi_SourceReadLine(PiSource *const source)
{
    size_t i = 0;

    do
    {
        const int c = getc(source->stream);

        if ((c == EOF) || (c == '\0') || (c == '\n'))
            break;

        source->buffer[i++] = (char)c;
    } while (i < (source->bufferSize - 2));

    source->buffer[i + 0] = '\n';
    source->buffer[i + 1] = '\0';

    /* Sets the current position to the beginning of the buffer */
    source->currentPos.offset = 0;
    /* Sets the forward position to the beginning of the buffer */
    source->forwardPos.offset = 0;

    return i;
}

static inline int pi_SourceReadRepl(PiSource *const source)
{
    int result;

    if (pi_SourceReadLine(source) > 0)
        result = pi_SourceRead(source);
    else
        result = EOF;

    return result;
}

int piSourceRead(PiSource *const source)
{
    assert(source != NULL);

    if (!pi_SourceIsOpen(source))
        piRaiseError("tried to read from a closed source stream", NULL);

    int result;

    if (source->forwardPos.offset >= source->bufferSize)
    {
        switch (source->kind)
        {
        case PI_SOURCE_KIND_TEXT:
            result = EOF;
            break;
        case PI_SOURCE_KIND_FILE:
            result = pi_SourceReadFile(source);
            break;
            /*
        case PI_SOURCE_KIND_REPL:
            result = pi_SourceReadRepl(source);
            break;
            */

        default:
            piUnreachable();
            break;
        }
    }
    else
    {
        /* Reads the character */
        result = pi_SourceRead(source);
    }

    return result;
}

void piSourceAdvance(PiSource *const source)
{
    assert(source != NULL);

    /* Moves the forward position to the current position */
    source->currentPos = source->forwardPos;

    return;
}

void piSourceRetreat(PiSource *const source)
{
    assert(source != NULL);

    /* Moves the forward position to the current position */
    source->forwardPos = source->currentPos;

    return;
}

/**
 * +---- REPL Specific Functions ----------+
 */

PiSource *piOpenReplStream(PiSource *const source, const char *const name, FILE *const stream, const size_t maxLine)
{
    assert(source != NULL && stream != NULL && maxLine > 0);

    /* Sets naming info */
    source->name = pi_StringOrEmpty(name);
    source->path = NULL;
    
    /* Sets stream typology */
    source->kind = PI_SOURCE_KIND_REPL;

    /* Sets stream info */
    source->stream = stream;
    source->streamSize = 0;

    /* Initializes a new buffer */
    return pi_InitSourceBuffer(source, NULL, maxLine);
}

size_t piReplReadLine(PiSource *const source)
{
    assert(source != NULL);

    if (!pi_SourceIsOpen(source))
        piRaiseError("tried to read from a closed source stream", NULL);

    return pi_SourceReadLine(source);
}

/**
 * +---- SourceSpan -----------------------+
 */

PiSourceSpan *piInitSourceSpan(PiSourceSpan *const sourceSpan, const PiSource *const source, const PiSourceSpanningMode spanningMode)
{
    assert(source != NULL);

    PiSourceSpan *result;

    if (sourceSpan)
        result = sourceSpan;
    else
        result = PI_New(PiSourceSpan);

    result->source = source;

    switch (spanningMode)
    {
    case PI_SOURCE_SPAN_LEXEME:
        result->text = strdup(piSourceGetRawLexeme(source));
        result->length = piSourceGetLexemeLength(source);
        /* Information about the position of the lexeme */
        result->offset = source->currentPos.offset;
        result->line = source->currentPos.line;
        result->column = source->currentPos.column;
        break;
    case PI_SOURCE_SPAN_LINE:
        result->text = strdup(piSourceGetRawLine(source));
        result->length = piSourceGetLineLength(source);
        /* Information about the starting position of the line */
        result->offset = 0;
        result->line = source->currentPos.line;
        result->column = source->currentPos.column;
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

PiSourceSpan *piFreeSourceSpan(PiSourceSpan *const sourceSpan)
{

}

/* =------------------------------------------------------------= */
