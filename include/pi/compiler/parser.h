#pragma once

/**
 * @file        parser.h
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
 * @brief       Syntax analyzer (placeholder for future implementation).
 */

#ifndef _PI_COMPILER_PARSER_H
#define _PI_COMPILER_PARSER_H

#include "pi/compiler/lexer.h"

#include "pi/svm/chunk.h"
#include "pi/svm/opcode.h"

PI_C_HEADER_BEGIN

/* =---- Semantic Analysis -------------------------------------= */

/**
 * +---- Parser ---------------------------+
 */

typedef struct _pi_Parser
{
    PiToken curr;
    PiToken prev;
} PiParser;



/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
