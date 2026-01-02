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

#ifndef _PI_SVM_DISASM_H
#define _PI_SVM_DISASM_H

#include "pi/svm/chunk.h"

#include <stdio.h>

PI_C_HEADER_BEGIN

/* =---- Stack-VM Disassembler ---------------------------------= */

/**
 * @brief   Writes the disassembled chunk into a specific file stream.
 * 
 * @param[in] stream    The stream to which write.
 * @param[in] chunk     The chunk to disassemble.
 * @param     name      The name of the chunk.
 */
void piDisasmSvmChunk(FILE *const stream, const PiSvmChunk *const chunk, const char *const name);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
