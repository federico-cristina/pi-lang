#pragma once

/**
 * @file        lexer.h
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
 * @brief       Lexical analyzer with keyword recognition and token scanning.
 */

#ifndef _PI_COMPILER_LEXER_H
#define _PI_COMPILER_LEXER_H

#include "pi/compiler/source.h"
#include "pi/compiler/token.h"

PI_C_HEADER_BEGIN

/* =---- Lexical Analysis --------------------------------------= */

typedef struct _pi_Keyword
{
#ifdef PI_USE_COLORS
    const char *color;
#endif
    const char *text;
    uint32_t    length;
    PiTokenCode code;
} PiKeyword;

bool piGetKeyword(PiKeyword *const keyword, const char *const start, const uint32_t length);

/**
 * +---- Scanner --------------------------+
 */

PiTokenCode piScanToken(PiSource *const source, PiToken *const outToken);


/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
