#pragma once

/**
 * @file        string.h
 *
 * @author      Federico Crisitina <federico.cristina@outlook.it>
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
 * @brief       String type and operations.
 */

#ifndef _PI_RUNTIME_STRING_H
#define _PI_RUNTIME_STRING_H

#include "pi/common/common.h"
#include "pi/runtime/object.h"
#include "pi/runtime/gc.h"

PI_C_HEADER_BEGIN

/* =---- Strings -----------------------------------------------= */

/* Forward declarations */
typedef struct _pi_Environment PiEnv;

/**
 * +---- String Allocation ----------------+
 */

/**
 * @brief   Allocate a new string object from C string.
 *
 * Allocates a string object and copies the provided C string data.
 * The string data is stored inline with the object header for cache
 * efficiency. The hash is computed during allocation using FNV-1a.
 *
 * @param[in,out] env    Environment context containing GC state.
 * @param[in]     cstr   Null-terminated C string to copy.
 * @param[in]     length Length of string (excluding null terminator).
 *
 * @return New string object with refcount = 1.
 *
 * @note The caller owns the returned string. Use piGcRetain() to add
 *       references or piGcRelease() to release ownership.
 *
 * @note The cstr parameter is copied; the caller retains ownership.
 *
 * @warning The length parameter must match the actual string length.
 *          Mismatches can lead to buffer overruns or truncated strings.
 *
 * @example
 *     PiStringObject *str = piStringNew(env, "Hello", 5);
 *     // ... use string ...
 *     piGcRelease(env, &str->base);
 */
PI_Api(PiStringObject *) piStringNew(PiEnv *const env, const char *const cstr, const uint32_t length);
/**
 * @brief   Allocate a string from format string (like sprintf).
 *
 * Allocates a string object and formats it using printf-style formatting.
 * This is useful for constructing strings from multiple values.
 *
 * @param[in,out] env    Environment context.
 * @param[in]     format Printf-style format string.
 * @param[in]     ...    Arguments for format string.
 *
 * @return New formatted string object with refcount = 1.
 *
 * @note Uses vsnprintf internally for safe formatting.
 *
 * @example
 *     PiStringObject *str = piStringFormat(env, "Value: %d", 42);
 *     // str->data contains "Value: 42"
 *     piGcRelease(env, &str->base);
 */
PI_Api(PiStringObject *) piStringFormat(PiEnv *const env, const char *const format, ...);

/**
 * +---- String Accessors -----------------+
 */

/**
 * @brief   Get C string pointer (read-only).
 *
 * Returns a pointer to the null-terminated C string data.
 * The returned pointer is valid for the lifetime of the string object.
 *
 * @param[in] str String object (may be NULL).
 *
 * @return Pointer to null-terminated C string, or "" if str is NULL.
 *
 * @note The returned pointer is read-only. Do not modify the data.
 *
 * @note NULL-safe: returns empty string if str is NULL.
 *
 * @example
 *     const char *cstr = piStringCStr(str);
 *     printf("String: %s\n", cstr);
 */
PI_InlineApi(const char *) piStringCStr(const PiStringObject *const str)
{
    return str ? str->data : "";
}
/**
 * @brief   Get string length in bytes.
 *
 * Returns the length of the string in bytes, excluding the null terminator.
 *
 * @param[in] str String object (may be NULL).
 *
 * @return Length in bytes (excluding null terminator), or 0 if str is NULL.
 *
 * @note NULL-safe: returns 0 if str is NULL.
 *
 * @note This returns the byte length, not the character count. For UTF-8
 *       strings, the byte length may be greater than the character count.
 *
 * @example
 *     uint32_t len = piStringLength(str);
 *     printf("Length: %u\n", len);
 */
PI_InlineApi(uint32_t) piStringLength(const PiStringObject *const str)
{
    return str ? str->length : 0;
}
/**
 * @brief   Get cached hash code.
 *
 * Returns the pre-computed hash code for the string. The hash is computed
 * during string allocation using the FNV-1a algorithm.
 *
 * @param[in] str String object (may be NULL).
 *
 * @return Hash code, or 0 if str is NULL.
 *
 * @note NULL-safe: returns 0 if str is NULL.
 *
 * @note The hash is computed once during allocation and cached for
 *       O(1) hash table lookups.
 *
 * @example
 *     uint32_t hash = piStringHash(str);
 */
PI_InlineApi(uint32_t) piStringHash(const PiStringObject *const str)
{
    return str ? str->hash : 0;
}

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
