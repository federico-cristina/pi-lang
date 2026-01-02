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

#ifndef _PI_SVM_OPCODE_H
#define _PI_SVM_OPCODE_H

#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- Stack-VM OpCodes Definition ---------------------------= */

/**
 * @brief   List of opcodes used by the Stack-VM (SVM) to represent operations.
 * 
 * @note    Since the SVM is an evaluation stack-oriented version of the virtual machine,
 *          the opcodes and program architecture also differ from the register-oriented
 *          version, so programs compiled for one bytecode will not be compatible with
 *          programs compiled for the other bytecode.
 * 
 *          The reason the SVM exists is to be able to test the language in its early stages
 *          even without a super-efficient virtual machine. Simply put, it simplifies the
 *          initial implementation and offloads the complex tasks to the future, while keeping
 *          in mind that the original design uses register-oriented virtual machines.
 */
typedef enum _pi_svm_OpCode
{
    /**
     * @brief   Represents an invalid opcode; useful for reporting errors or propagating them.
     */
    PI_SVM_OP_INVALID = -1,

#ifndef PI_DefineOp
#   define PI_DefineOp(name, code, ...) \
    PI_SVM_ ## name = code,
#endif

#include "opcode.inc"

#ifdef PI_DefineOp
#   undef PI_DefineOp
#endif

    /**
     * @brief   Represents the number of defined opcodes.
     */
    PI_SVM_OP_COUNT
} PiSvmOpCode;

/**
 * @brief   This function maps opcodes to a mnemonic string that represents them in the assembler
 *          and disassembler.
 */
static inline const char *piGetOpCodeName(const PiSvmOpCode opcode)
{
    const char *result;

    switch (opcode)
    {
    case PI_SVM_OP_INVALID:
        result = "invalid";
        break;

#ifndef PI_DefineOp
#   define PI_DefineOp(_, code, menmonic, ...)  \
    case code:                                  \
        result = menmonic;                      \
        break;
#endif

#include "opcode.inc"

#ifdef PI_DefineOp
#   undef PI_DefineOp
#endif
    
    default:
        piRaiseError("uknown opcode " PI_EscapedCharColor("%02" PRIX8), (uint8_t)opcode);
        break;
    }

    return result;
}

#if PI_USE_COLORS
/**
 * @brief   This function maps opcodes to a color string that represents them in the assembler
 *          and disassembler.
 */
static inline const char *piGetOpCodeColor(const PiSvmOpCode opcode)
{
    const char *result;

    switch (opcode)
    {
    case PI_SVM_OP_INVALID:
        result = PI_BOLD_RED;
        break;

#ifndef PI_DefineOp
#   define PI_DefineOp(_, code, __, color, ...) \
    case code:                                  \
        result = color;                         \
        break;
#endif

#include "opcode.inc"

#ifdef PI_DefineOp
#   undef PI_DefineOp
#endif
    
    default:
        piRaiseError("uknown opcode " PI_EscapedCharColor("%02" PRIX8), (uint8_t)opcode);
        break;
    }

    return result;
}
#endif

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
