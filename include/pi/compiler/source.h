#pragma once

/**
 * @file        source.h
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
 * @brief       Source stream abstraction for text, files, and REPL input.
 */

#ifndef _PI_COMPILER_SOURCE_H
#define _PI_COMPILER_SOURCE_H

#include "pi/common/common.h"

#include <stdlib.h>
#include <stdio.h>

PI_C_HEADER_BEGIN

/* =---- Streams of Source Code --------------------------------= */

/**
 * @brief   Represents a record of information about the location of a stream at
 *          a specific point.
 */
typedef struct _pi_SourcePosition
{
    /**
     * @brief   Represents the offset from the start of the stream to the current
     *          position.
     */
    uint64_t offset;
    /**
     * @brief   Represents the line number at the current position.
     */
    uint32_t line;
    /**
     * @brief   Represents the column number at the current position.
     */
    uint32_t column;
} PiSourcePos;

/**
 * @brief   This function resets the specified position marker to its initial value.
 * 
 * @param[in] pos   A pointer to the position marker to reset or initialize.
 */
PI_InlineApi(void) piResetSourcePosition(PiSourcePos *const pos)
{
    assert(pos != NULL);

    pos->offset = 0;
    pos->line = 1;
    pos->column = 1;

    return;
}

/**
 * +---- Source ---------------------------+
 */

/**
 * @brief   Represents the textual encoding format of a source stream.
 */
typedef enum _pi_SourceEncoding
{
    /**
     * @brief   Specifies that a source stream has ASCII characters.
     */
    PI_SOURCE_ENCODING_ASCII,
} PiSourceEncoding;

/**
 * @brief   Represents the typology of input of a source stream.
 */
typedef enum _pi_SourceKind
{
    /**
     * @brief   When a source stream has this flag set it is treated as a stream that
     *          has as its source a fixed-length text string.
     * 
     *          Once the end of the specified string is reached, `EOF` will be returned.
     */
    PI_SOURCE_KIND_TEXT,
    /**
     * @brief   When a source stream has this flag set it is treated as a non-interactive
     *          stream from an open file, which will be closed when the stream is closed.
     */
    PI_SOURCE_KIND_FILE,
    /**
     * @brief   When a source stream has this flag set it is treated as an interactive
     *          stream (usually originating from stdin).
     */
    PI_SOURCE_KIND_REPL,
} PiSourceKind;

/**
 * @brief   It represents the information record for managing a stream (from text or
 *          file) and using it as a source for the source code.
 * 
 * @note    *NEVER* directly manipulate the fields of this structure.
 */
typedef struct _pi_Source
{
    /**
     * @brief   Represents the filename or alias given to the source string from which
     *          the source stream was created.
     */
    const char         *name;
    /**
     * @brief   Represents the full path (absolute or relative) used to open the source
     *          file.
     *          
     *          This field is set to `NULL` if a file is not used as the source.
     */
    const char         *path;
    /**
     * @brief   Represents a pointer to the source `FILE`.
     * 
     *          This field is set to `NULL` if a file is not used as the source.
     */
    FILE               *stream;
    /**
     * @brief   Represents a buffer used as a text source or as an intermediate buffer
     *          when reading blocks of a file larger than a certain number of bytes.
     */
    char               *buffer;
    /**
     * @brief   Represents the size (in bytes) of the file used as source.
     */
    size_t              streamSize;
    /**
     * @brief   Represents the size (in bytes) of the buffer, therefore its maximum capacity.
     */
    size_t              bufferSize;
    /**
     * @brief   Represents the difference between the start of the source file and the start
     *          of the buffer.
     */
    uint64_t            delta;
    /**
     * @brief   Represents which kind of source stream is the current one.
     */
    PiSourceKind        kind;
    /**
     * @brief   The encoding format of the input text.
     * 
     * @note    As now, the only text encoding supporte is ASCII and it will be for a looooong
     *          time. This field exist for future implementations.
     */
    PiSourceEncoding    encoding;
    /**
     * @brief   It represents the current position marker, that is, the starting position of
     *          the lexeme being scanned.
     * 
     * @note    This is a feature directly related to the lexical analyzer (specifically the
     *          tokenizer).
     */
    PiSourcePos         currentPos;
    /**
     * @brief   It represents the forward position marker, that is, the final position of
     *          the lexeme being scanned.
     * 
     * @note    This is a feature directly related to the lexical analyzer (specifically the
     *          tokenizer).
     */
    PiSourcePos         forwardPos;
} PiSource;

/**
 * @brief   This function opens a source stream using a buffer as a base.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream to buffer.
 * @param[in] name      The alias to give to the buffer.
 * @param[in] buffer    A pointer to the character buffer used as the source stream. When this parameter
 *                      is NULL, a new buffer with a maximum capacity of `size` is allocated.
 * @param     size      The size of the buffer.
 * 
 * @return  This function returns a pointer to the stream where the specified source was opened.
 */
PiSource *piOpenSourceBuffer(PiSource *const source, const char *const name, char *const buffer, const size_t size);
/**
 * @brief   This function opens a source stream using a `FILE` stream as a base.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream to buffer.
 * @param[in] name      The alias to give to the stream.
 * @param[in] stream    A pointer to the `FILE` stream used as a base.
 * 
 * @return  This function returns a pointer to the stream where the specified source was opened.
 */
PiSource *piOpenSourceStream(PiSource *const source, const char *const name, FILE *const stream);
/**
 * @brief   This function opens a source stream using a raw string as a base.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream to buffer.
 * @param[in] name      The alias to give to the string.
 * @param[in] text      A pointer to the source text.
 * 
 * @return  This function returns a pointer to the stream where the specified source was opened.
 */
PiSource *piOpenSourceText(PiSource *const source, const char *const name, const char *const text);
/**
 * @brief   This function opens a source stream using a file stream as a base.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream to buffer.
 * @param[in] path      The path to the file to open.
 * 
 * @return  This function returns a pointer to the stream where the specified source was opened.
 */
PiSource *piOpenSourceFile(PiSource *const source, const char *const path);

/**
 * @brief   This function closes a previously opened source stream.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream.
 * 
 * @return  This function returns a pointer to the closed source stream.
 */
PiSource *piCloseSource(PiSource *const source);

/**
 * @brief   Peeks the next value from a PiSource without advancing its read position.
 * 
 * @param[in] source    Pointer to the PiSource to peek into. The pointer and the pointed-to
 *                      object are const; the function does not modify the source.
 * 
 * @return  The next value from the source as an int. A negative value typically indicates
 *          end-of-source or an error (for example EOF).
 */
int piSourcePeek(const PiSource *const source);
/**
 * @brief   Reads data from a PiSource.
 * 
 * @param[in] source    Pointer to the PiSource to read from.
 * 
 * @return  An integer indicating the result of the read operation (for example, the number of bytes
 *          read on success or a negative error code on failure).
 */
int piSourceRead(PiSource *const source);

/**
 * @brief   This function moves the lexeme's start position marker to the end position marker, thus
 *          marking the beginning of the new lexeme.
 * 
 * @param[in] source    Pointer to the PiSource to advance. Must point to a valid, initialized PiSource
 *                      (must not be NULL).
 */
void piSourceAdvance(PiSource *const source);
/**
 * @brief   This function moves the lexeme's end position marker to the start position marker, restarting
 *          the lexeme scan from the beginning.
 * 
 * @param[in] source    Pointer to the PiSource to advance. Must point to a valid, initialized PiSource
 *                      (must not be NULL).
 */
void piSourceRetreat(PiSource *const source);

/**
 * @brief   Checks whether the value peeked from a PiSource equals the given value.
 * 
 * @param[in] source    Pointer to the PiSource to inspect (the source is not modified).
 * @param     c         Integer value to compare against the peeked value (typically a character
 *                      code).
 * 
 * @return  `true` if `piSourcePeek(source)` equals `c`; otherwise `false`.
 */
PI_InlineApi(bool) piSourceCheck(const PiSource *const source, const int c)
{
    return piSourcePeek(source) == c;
}
/**
 * @brief   Checks whether the given `PiSource` matches the specified integer value. If it matches,
 *          advances the source by calling `piSourceRead` and returns `true`; otherwise leaves the source
 *          unchanged and returns `false`.
 * 
 * @param[in] source    Pointer to the PiSource to check.
 * @param     c         Integer value (e.g., a character or token code) to compare against the source.
 * 
 * @return  `true` if the source matched the value and was advanced (`piSourceRead` called); `false` if there
 *          was no match.
 */
PI_InlineApi(bool) piSourceMatch(PiSource *const source, const int c)
{
    const bool result = piSourceCheck(source, c);

    if (result)
        piSourceRead(source);

    return result;
}

/**
 * @brief   Represents a predicate for a character recognition function.
 */
typedef bool (*PiPredicateFn)(const int);

/**
 * @brief   Invokes the given predicate on the value obtained from piSourcePeek(source) and returns the predicate's
 *          boolean result.
 * 
 * @param[in] source    Pointer to a const PiSource used to obtain a value for testing. The function does not modify
 *                      the source.
 * @param     predicate A predicate function (PiPredicateFn) that accepts the value returned by piSourcePeek and returns
 *                      a bool indicating whether the value satisfies the condition.
 * 
 * @return  The boolean result produced by calling predicate on the value returned from piSourcePeek(source).
 */
PI_InlineApi(bool) piSourceCheckPred(const PiSource *const source, const PiPredicateFn predicate)
{
    return predicate(piSourcePeek(source));
}
/**
 * @brief   Evaluates a predicate on the given source and, if the predicate matches, reads (consumes) the source.
 * 
 * @param[in] source    The PiSource to evaluate. If the predicate matches, this function will call piSourceRead(source)
 *                      to read or advance the source.
 * @param     predicate The predicate function used to test the source. If it returns true, the source will be read.
 * 
 * @return  The boolean result produced by calling predicate on the value returned from piSourcePeek(source).
 */
PI_InlineApi(bool) piSourceMatchPred(PiSource *const source, const PiPredicateFn predicate)
{
    const bool result = piSourceCheckPred(source, predicate);

    if (result)
        piSourceRead(source);

    return result;
}

/**
 * @brief   Returns a pointer to the beginning of the read lexeme.
 */
PI_InlineApi(const char *) piSourceGetRawLexeme(const PiSource *const source)
{
    return (const char *)&source->buffer[source->currentPos.offset];
}
/**
 * @brief   Returns the length of the read lexme.
 */
PI_InlineApi(uint32_t) piSourceGetLexemeLength(const PiSource *const source)
{
    return (uint32_t)(source->forwardPos.offset - source->currentPos.offset);
}

/**
 * @brief   Returns a pointer to the beginning of the line.
 */
PI_InlineApi(const char *) piSourceGetRawLine(const PiSource *const source)
{
    /* Starts from the beginning of the current lexeme */
    const char *p = piSourceGetRawLexeme(source);

    /* Searches for the beginning of the line */
    while (((p - source->buffer) > 0) && (*(p - 1) != '\n'))
        --p;

    /* Returns the beginning of the line */
    return p;
}
/**
 * @brief   Returns the length of the line.
 */
PI_InlineApi(uint32_t) piSourceGetLineLength(const PiSource *const source)
{
    const char
        /* An immutable pointer to the beginning of the line */
        *const pStart = piSourceGetRawLine(source),
        /* A mutable pointer that will scan the line */
        *p = pStart;

    /* Searches for the ending of the line */
    while ((((source->buffer + source->bufferSize) - p) > 0) && (*(p - 1) != '\n'))
        ++p;

    /* Returns the endind of the line */
    return (uint32_t)(p - pStart);
}

/**
 * +---- REPL Specific Functions ----------+
 */

/**
 * @brief   This function opens a source stream using an interactive `FILE` stream as a base.
 * 
 * @param[in] source    A pointer to the record containing the data for a source stream to buffer.
 * @param[in] name      The alias to give to the stream.
 * @param[in] stream    A pointer to the `FILE` stream used as a base.      
 * @param     maxLine   The maximum number of character that a line will be able to contain.
 * 
 * @return  This function returns a pointer to the stream where the specified source was opened.
 */
PiSource *piOpenReplStream(PiSource *const source, const char *const name, FILE *const stream, const size_t maxLine);

/**
 * @brief   This functions read a line from the interactive REPL stream.
 */
size_t piReplReadLine(PiSource *const source);

/**
 * +---- SourceSpan -----------------------+
 */

/**
 * @brief   Represents a span of source code.
 */
typedef struct _pi_SourceSpan
{
    /**
     * @brief   Represents a pointer to source information.
     */
    const PiSource *source;
    /**
     * @brief   Represents a pointer to the start of the span.
     */
    const char     *text;
    /**
     * @brief   Represents the length (in bytes) of the span of text.
     */
    uint32_t        length;
    /**
     * @brief   Represents the offset from the start of the stream.
     */
    uint32_t        offset;
    /**
     * @brief   Represents the line number of the span.
     */
    uint32_t        line;
    /**
     * @brief   Represents the column number of the span.
     */
    uint32_t        column;
} PiSourceSpan;

/**
 * @brief   Represents the modality with which are created source spans.
 */
typedef enum _pi_SourceSpanningMode
{
    /**
     * @brief   Creates a span from the beginning to the end of the current lexeme.
     */
    PI_SOURCE_SPAN_LEXEME,
    /**
     * @brief   Creates a span from the beginning to the end of the current line.
     */
    PI_SOURCE_SPAN_LINE,
} PiSourceSpanningMode;

PiSourceSpan *piInitSourceSpan(PiSourceSpan *const sourceSpan, const PiSource *const source, const PiSourceSpanningMode spanningMode);
PiSourceSpan *piFreeSourceSpan(PiSourceSpan *const sourceSpan);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
