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

#ifndef _PI_RUNTIME_ENV_H
#define _PI_RUNTIME_ENV_H

#include "pi/common/common.h"

#include "pi/runtime/object.h"
#include "pi/runtime/value.h"

PI_C_HEADER_BEGIN

/* =---- Execution Environments --------------------------------= */

#ifndef PI_ENV_HEAP_SIZE_MIN
#   define PI_ENV_HEAP_SIZE_MIN 0
#endif

#ifndef PI_ENV_HEAP_SIZE_MAX
#   define PI_ENV_HEAP_SIZE_MAX 0
#endif

/**
 * +---- Handlers -------------------------+
 */

typedef struct _pi_Environment PiEnv;

typedef void *(*PiMemAllocHandlerFn  )(PiEnv *const env, const size_t size);
typedef void *(*PiMemReallocHandlerFn)(PiEnv *const env, void *const block, const size_t newSize);
typedef void  (*PiMemFreeHandlerFn   )(PiEnv *const env, void *const block);

typedef struct _pi_Handlers
{
    PiMemAllocHandlerFn     alloc;
    PiMemReallocHandlerFn   realloc;
    PiMemFreeHandlerFn      free;
} PiHandlers;

/**
 * +---- Env ------------------------------+
 */

struct _pi_Environment
{
    /**
     * @brief   When this flag is `true` it means that this struct has alredy been
     *          initialized.
     */
    bool                    isInit;
    /**
     * @brief   When this flag is `true` it means that the program is running in REPL
     *          mode.
     */
    bool                    isRepl;
    /**
     * @brief   When this flag is `true` it means that the program sent an interrupt
     *          message to the system (typically from a Ctrl+C).
     */
    volatile bool           interrupt;
    /**
     * @brief   An integer representing the exit code that will be returned by this process.
     */
    volatile int            exitCode;
#ifdef _WIN32
    /**
     * @brief   Represents an handle to the heap structure where objects will be allocated.
     */
    void                   *heap;
#endif
    /**
     * @brief   Represents basic operations handler functions.
     */
    PiHandlers              handlers;
};

PiEnv *piInitEnv(PiEnv *const env);
PiEnv *piFreeEnv(PiEnv *const env);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
