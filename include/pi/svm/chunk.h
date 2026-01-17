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

#ifndef _PI_SVM_CHUNK_H
#define _PI_SVM_CHUNK_H

#include "pi/runtime/value.h"
#include "pi/svm/opcode.h"

PI_C_HEADER_BEGIN

/* =---- Chunks of Bytecode ------------------------------------= */

/**
 * @brief   This data structure represents instructions line information (encoded with an RLE model).
 */
typedef struct _pi_svm_LineInfo
{
    /**
     * @brief   Represents the previous line information.
     */
    struct _pi_svm_LineInfo *prev;
    /**
     * @brief   A slice of the line of the source code.
     */
    const char              *text;
    /**
     * @brief   Represents the number of the line.
     */
    uint32_t                 line;
    /**
     * @brief   Represents the maximum offset of the instructions of this line.
     */
    uint32_t                 offset;
} PiSvmLineInfo;

/**
 * +---- Chunk ----------------------------+
 */

/**
 * @brief   This data structure represents a single portion of source code compiled into bytecode.
 * 
 *          By 'portion' we mean not only the bytecode itself, as a collection of bytes, but also
 *          the static data used in that area and debugging information.
 */
typedef struct _pi_svm_Chunk
{
    /**
     * @brief   Represents the bytecode array.
     */
    uint8_t        *code;
    /**
     * @brief   Represents the number of bytes written to the array.
     */
    uint32_t        count;
    /**
     * @brief   Represents the maximum capacity of the array.
     */
    uint32_t        cap;
    /**
     * @brief   Represents the value constant pool.
     */
    PiValueArray    data;
    /**
     * @brief   Represents source line informations.
     */
    PiSvmLineInfo  *lines;
} PiSvmChunk;

/**
 * @brief   This function takes care of initializing (without allocating resources, yet) a chunk.
 */
PiSvmChunk *piInitSvmChunk(PiSvmChunk *const chunk);
/**
 * @brief   This function is responsible for releasing the resources used by a chunk.
 */
PiSvmChunk *piFreeSvmChunk(PiSvmChunk *const chunk);

/**
 * @brief   This functions writes source line code information.
 */
void piSvmChunkPushLine(PiSvmChunk *const chunk, const uint32_t lineNumber, const char *const line);

/**
 * @brief   This function writes an operation into the chunk that requires only the opcode.
 */
void piSvmChunkWriteOp(PiSvmChunk *const chunk, const PiSvmOpCode opcode);
/**
 * @brief   This function writes an operation into the chunk that requires the opcode and an argument.
 */
void piSvmChunkWriteShortOp(PiSvmChunk *const chunk, const PiSvmOpCode opcode, const uint8_t arg);

/**
 * @brief   This function pushes a constant value into the constant values pool.
 */
static inline uint32_t piSvmChunkAddConst(PiSvmChunk *const chunk, const PiValue value)
{
    return piValueArrayPush(&chunk->data, value);
}

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
