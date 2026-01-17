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

#ifndef _PI_SVM_SVM_H
#define _PI_SVM_SVM_H

#include "pi/runtime/env.h"
#include "pi/runtime/value.h"

#include "pi/svm/chunk.h"

PI_C_HEADER_BEGIN

/* =---- Stack-based Virtual Machine ---------------------------= */

/**
 * @brief   This function executes a compile chunk of bytecode in a specific virtual environment.
 */
int piRunSvmChunk(PiEnv *const env, const PiSvmChunk *const chunk);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
