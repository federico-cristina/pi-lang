#pragma once

/**
 * @brief       This file contains all the directives for including the various main
 *              application interface (API) files and the definitions for the most
 *              significant and useful helper functions.
 * 
 *              If you have any suggestions or requests for expanding the main API,
 *              please contact me at federico.cristina@outlook.it.
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
 */

#ifndef _PI_H
#define _PI_H

/* Include shared definitions */
#include "pi/common/common.h"

/**
 * +---- Runtime Headers ------------------+
 */

/* Include runtime value and value array structures */
#include "pi/runtime/value.h"
/* Include environments managagement */
#include "pi/runtime/env.h"

/**
 * +---- Stack-VM Headers -----------------+
 */

/* Include stack-oriented opcode definitions */
#include "pi/svm/opcode.h"
/* Include chunks of stack-oriented bytecode */
#include "pi/svm/chunk.h"
/* Include stack-vm execution loop */
#include "pi/svm/svm.h"

/* Include stack-oriented bytecode disassembler */
#include "pi/svm/disasm.h"

/**
 * +---- Compiler Headers -----------------+
 */

/* Include streams of source code management */
#include "pi/compiler/source.h"
/* Include lexical anlysis tools */
#include "pi/compiler/lexer.h"

/**
 * +---------------------------------------+
 */

#endif
