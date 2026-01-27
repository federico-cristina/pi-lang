/**
 * @file        diagnostic.c
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
 * @brief       Diagnostic system implementation.
 */

#include "pi/compiler/diagnostic.h"

/* =---- Diagnostic System -------------------------------------= */

typedef struct _pi_DiagnosticInfo
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
} PiDiagnosticInfo;

static inline const PiDiagnosticInfo pi_GetDiagnosticInfo(const PiError error)
{
    static const PiDiagnosticInfo pi_DiagnosticsTable[PI_ERROR_COUNT + 1] =
    {
#ifndef PI_DefineError
#   define PI_DefineError(name, code, level, kind, fmt, ...)    \
        [PI_ ## kind ## _ ## name] = {                          \
            .errorCode = code,                                  \
            .errorKind = PI_ERROR_KIND_ ## kind,                \
            .severity = PI_SEVERITY_LEVEL_ ## level,            \
            .format = fmt,                                      \
        },
#endif
    
#include _PI_COMPILER_DIAGNOSTIC_INC

        {
            .errorCode = PI_ERROR_CODE_INVALID,
            .errorKind = PI_ERROR_KIND_ERROR,
            .severity = PI_SEVERITY_LEVEL_FATAL,
            .format = NULL,
        }
    };

    if ((error > PI_ERROR_INVALID) && (error < PI_ERROR_COUNT))
        return pi_DiagnosticsTable[error];
    else
        return pi_DiagnosticsTable[PI_ERROR_COUNT];
}

/**
 * +---- Diagnostic -----------------------+
 */

PiDiagnostic *piInitDiagnostic(PiDiagnostic *const diagnostic, const PiError error)
{
    assert(diagnostic != NULL);

    const PiDiagnosticInfo diagnosticInfo = pi_GetDiagnosticInfo(error);

    diagnostic->errorCode = diagnosticInfo.errorCode;
    diagnostic->errorKind = diagnosticInfo.errorKind;
    diagnostic->severity = diagnosticInfo.severity;
    diagnostic->format = diagnosticInfo.format;

    diagnostic->reports = NULL;
    diagnostic->next = NULL;

    return diagnostic;
}

PiDiagnostic *piFreeDiagnostic(PiDiagnostic *const diagnostic)
{
    assert(diagnostic != NULL);

    if (diagnostic->reports)
        ; /* ToDo: Free reports */

    if (diagnostic->next)
        ; /* ToDo: Free next */

    return piInitDiagnostic(diagnostic, PI_ERROR_INVALID);
}

/**
 * +---- DiagnosticManager ----------------+
 */



/* =------------------------------------------------------------= */
