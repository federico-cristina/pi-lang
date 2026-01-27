#pragma once

/**
 * @file        token.h
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
 * @brief       Lexical tokens with source position tracking.
 */

#ifndef _PI_COMPILER_TOKEN_H
#define _PI_COMPILER_TOKEN_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Lexical Tokens ----------------------------------------= */

/**
 * +---- TokenCode ------------------------+
 */

typedef enum _pi_TokenCode
{
    PI_TOKEN_INVAL = -1,

#ifndef PI_DefineToken
#   define PI_DefineToken(name, ...) \
    PI_TOKEN_ ## name,
#endif

#include "token.inc"

    PI_TOKEN_COUNT
} PiTokenCode;

/**
 * +---- Token ----------------------------+
 */

typedef struct _pi_Token
{
    const char *source;
    const char *text;
    uint32_t    length;
    uint32_t    line;
    uint32_t    column;
    uint16_t    radix;
    PiTokenCode code    : 16;
} PiToken;

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
