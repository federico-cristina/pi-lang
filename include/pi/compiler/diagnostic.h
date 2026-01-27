#pragma once

/**
 * @file        diagnostic.h
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
 * @brief       Compiler diagnostic and error reporting system.
 */

#ifndef _PI_COMPILER_DIAGNOSTIC_H
#define _PI_COMPILER_DIAGNOSTIC_H

#include "pi/compiler/source.h"

PI_C_HEADER_BEGIN

/* =---- Diagnostic System -------------------------------------= */

typedef enum _pi_SeverityLevel
{
    PI_SEVERITY_LEVEL_SUPPRESSED = -3,
    PI_SEVERITY_LEVEL_DEPRECATED = -2,
    PI_SEVERITY_LEVEL_DISABLED = -1,
    PI_SEVERITY_LEVEL_NOTE =  0,
    PI_SEVERITY_LEVEL_DEBUG =  1,
    PI_SEVERITY_LEVEL_WARNING =  2,
    PI_SEVERITY_LEVEL_ERROR =  3,
    PI_SEVERITY_LEVEL_FATAL =  4,
} PiSeverityLevel;

#ifndef _PI_COMPILER_DIAGNOSTIC_INC
#   define _PI_COMPILER_DIAGNOSTIC_INC "pi/compiler/diagnostic.inc"
#endif

typedef enum _pi_Error
{
    PI_ERROR_INVALID = -1,

#ifndef PI_DefineError
#   define PI_DefineError(name, code, level_, kind, ...) \
    PI_ ## kind ## _ ## name,
#endif

#include _PI_COMPILER_DIAGNOSTIC_INC

    PI_ERROR_COUNT
} PiError;

typedef enum _pi_ErrorCode
{
    PI_ERROR_CODE_INVALID = -1,

#ifndef PI_DefineError
#   define PI_DefineError(name, code, ...) \
    PI_ERROR_CODE_ ## name = code,
#endif
    
#include _PI_COMPILER_DIAGNOSTIC_INC
} PiErrorCode;

typedef enum _pi_ErrorKind
{
    PI_ERROR_KIND_ERROR,
    PI_ERROR_KIND_SYNTAX_ERROR,
} PiErrorKind;

/**
 * +---- Diagnostic -----------------------+
 */

typedef struct _pi_Diagnostic       PiDiagnostic;
typedef struct _pi_DiagnosticReport PiDiagnosticReport;

struct _pi_DiagnosticReport
{
    const char         *message;
    const char         *label;
    PiSourceSpan        span;
    PiDiagnostic       *descriptor;
    PiDiagnosticReport *next;
};

typedef void (*PiDiagnosticTrapFn)(PiDiagnosticReport *const report);

struct _pi_Diagnostic
{
#if PI_DEBUG
    PiErrorCode         errorCode;
    PiErrorKind         errorKind;
#else
    unsigned short      errorCode;
    unsigned short      errorKind;
#endif
    PiSeverityLevel     severity;
    const char         *format;
    PiDiagnosticReport *reports;
    PiDiagnostic       *next;
};

PiDiagnostic *piInitDiagnostic(PiDiagnostic *const diagnostic, const PiError error);
PiDiagnostic *piFreeDiagnostic(PiDiagnostic *const diagnostic);

/**
 * +---- DiagnosticManager ----------------+
 */

typedef struct _pi_DiagnosticManager
{
    PiSeverityLevel     severityFilter;
} PiDiagnosticManager;

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
