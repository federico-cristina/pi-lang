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
 * @brief       Lightweight string type with pre-computed hash for use in hash tables.
 */

#ifndef _PI_COMMON_STRING_H
#define _PI_COMMON_STRING_H

/* Include common definitions */
#include "pi/common/common.h"

PI_C_HEADER_BEGIN

/* =---- FNV-1a Hash Function ----------------------------------= */

#ifndef PI_FNV_BASIS
/**
 * @brief   FNV-1a offset basis for 32-bit hash.
 */
#   define PI_FNV_BASIS 2166136261u
#endif

#ifndef PI_FNV_PRIME
/**
 * @brief   FNV-1a prime multiplier for 32-bit hash.
 */
#   define PI_FNV_PRIME 16777619u
#endif

/**
 * @brief   Compute FNV-1a hash for a byte sequence.
 *
 * @param[in] data    Pointer to data bytes.
 * @param[in] length  Number of bytes to hash.
 *
 * @return  32-bit FNV-1a hash value.
 */
PI_Api(uint32_t) piHashString(const char *const data, const uint32_t length);

/* =---- String ------------------------------------------------= */

/**
 * @brief   Lightweight string type with pre-computed hash.
 *
 *          This structure can reference either static string data (clear=0)
 *          or dynamically allocated data (clear=1). When clear=1, the
 *          chars pointer is owned and should be freed with piFree().
 */
typedef struct _pi_String
{
    /**
     * @brief   Pointer to null-terminated string data.
     */
    const char *chars;
    /**
     * @brief   Length of the string (excluding null terminator).
     */
    uint32_t    count : 31;
    /**
     * @brief   Ownership flag: 1 = dynamically allocated, 0 = static/external.
     */
    uint32_t    clear :  1;
    /**
     * @brief   Pre-computed FNV-1a hash code.
     */
    uint32_t    hash;
} PiString;

/**
 * +---- String::Init ---------------------+
 */

/**
 * @brief   Initialize a PiString from a static/external C string.
 *
 *          The PiString does NOT take ownership of the string data (clear=0).
 *          The length is computed using strlen() and the hash is computed
 *          using FNV-1a.
 *
 * @param[out] str   PiString to initialize.
 * @param[in]  cstr  Null-terminated C string (must not be NULL).
 *
 * @return  Pointer to the initialized PiString (same as str).
 */
PI_Api(PiString *) piInitString(PiString *const str, const char *const cstr);
/**
 * @brief   Initialize a PiString from a static/external C string with known length.
 *
 *          The PiString does NOT take ownership of the string data (clear=0).
 *          The hash is computed using FNV-1a.
 *
 * @param[out] str     PiString to initialize.
 * @param[in]  cstr    Pointer to string data (must not be NULL).
 * @param[in]  length  Length of the string (excluding null terminator).
 *
 * @return  Pointer to the initialized PiString (same as str).
 */
PI_Api(PiString *) piInitStringWithLength(PiString *const str, const char *const cstr, const uint32_t length);

/**
 * @brief   Initialize a PiString that owns dynamically allocated string data.
 *
 *          The PiString takes ownership of cstr (clear=1). The length is
 *          computed using strlen() and the hash is computed using FNV-1a.
 *          The string data will be freed when piFreeString() is called.
 *
 * @param[out] str   PiString to initialize.
 * @param[in]  cstr  Dynamically allocated string (must not be NULL).
 *
 * @return  Pointer to the initialized PiString (same as str).
 */
PI_Api(PiString *) piInitOwnedString(PiString *const str, char *const cstr);
/**
 * @brief   Initialize a PiString that owns dynamically allocated string data
 *          with known length.
 *
 *          The PiString takes ownership of cstr (clear=1). The hash is
 *          computed using FNV-1a. The string data will be freed when
 *          piFreeString() is called.
 *
 * @param[out] str     PiString to initialize.
 * @param[in]  cstr    Dynamically allocated string (must not be NULL).
 * @param[in]  length  Length of the string (excluding null terminator).
 *
 * @return  Pointer to the initialized PiString (same as str).
 */
PI_Api(PiString *) piInitOwnedStringWithLenght(PiString *const str, char *const cstr, const uint32_t length);

/**
 * +---- String::Copy ---------------------+
 */

/**
 * @brief   Create a shallow copy of a PiString.
 *
 *          Allocates a new PiString and copies all fields from str,
 *          including the chars pointer. Both strings will point to the
 *          same underlying data.
 *
 * @param[in] str  PiString to copy (must not be NULL).
 *
 * @return  Pointer to newly allocated PiString copy.
 *
 * @warning If str has clear=1, both the original and copy will have clear=1,
 *          which may lead to double-free. Use piStringDup() for owned strings.
 */
PI_Api(PiString *) piStringCopy(const PiString *const str);
/**
 * @brief   Shallow copy a PiString to an existing destination.
 *
 *          Copies all fields from str to dest. If dest is NULL, allocates
 *          a new PiString.
 *
 * @param[in,out] dest  Destination PiString (or NULL to allocate).
 * @param[in]     str   PiString to copy (must not be NULL).
 *
 * @return  Pointer to dest (or newly allocated PiString if dest was NULL).
 */
PI_Api(PiString *) piStringCopyTo(const PiString *const dest, const PiString *const str);

/**
 * +---- String::Dup ----------------------+
 */

/**
 * @brief   Create a deep copy of a PiString with owned data.
 *
 *          Allocates a new PiString and a new buffer for the string data.
 *          The returned PiString always has clear=1 (owns the data).
 *
 * @param[in] str  PiString to duplicate (must not be NULL).
 *
 * @return  Pointer to newly allocated PiString with its own copy of the data.
 */
PI_Api(PiString *) piStringDup(const PiString *const str);
/**
 * @brief   Deep copy a PiString to an existing destination.
 *
 *          Allocates a new buffer for the string data and copies it.
 *          If dest is NULL, allocates a new PiString. The result always
 *          has clear=1 (owns the data).
 *
 * @param[in,out] dest  Destination PiString (or NULL to allocate).
 * @param[in]     str   PiString to duplicate (must not be NULL).
 *
 * @return  Pointer to dest (or newly allocated PiString if dest was NULL).
 */
PI_Api(PiString *) piStringDupTo(const PiString *const dest, const PiString *const str);

/**
 * +---- String::Free ---------------------+
 */

/**
 * @brief   Free owned string data and reset the PiString.
 *
 *          If clear=1, frees the chars buffer using piFree().
 *          Always resets all fields to zero/NULL.
 *          Safe to call on any PiString, including NULL.
 *
 * @param[in,out] str  PiString to free (may be NULL).
 *
 * @return  The same pointer passed in (for chaining).
 */
PI_Api(PiString *) piFreeString(PiString *const str);

/**
 * +---- String::Equals -------------------+
 */

/**
 * @brief   Compare two PiStrings for equality.
 *
 *          Compares in order: hash, length, then actual characters.
 *          This provides fast rejection for non-equal strings.
 *
 * @param[in] a  First string (must not be NULL).
 * @param[in] b  Second string (must not be NULL).
 *
 * @return  true if strings are equal, false otherwise.
 */
PI_Api(bool) piStringEquals(const PiString *const a, const PiString *const b);
/**
 * @brief   Compare two PiStrings for equality, ignoring case.
 *
 *          Compares length first, then characters case-insensitively.
 *          Note: hash comparison is skipped since hashes are case-sensitive.
 *
 * @param[in] a  First string (must not be NULL).
 * @param[in] b  Second string (must not be NULL).
 *
 * @return  true if strings are equal ignoring case, false otherwise.
 */
PI_Api(bool) piStringEqualsIgnoreCase(const PiString *const a, const PiString *const b);

/**
 * +---- String::Compare ------------------+
 */

/**
 * @brief   Lexicographically compare two PiStrings.
 *
 *          Compares characters up to the length of the shorter string.
 *          If the prefixes are equal, the shorter string is considered
 *          less than the longer one.
 *
 * @param[in] a  First string (must not be NULL).
 * @param[in] b  Second string (must not be NULL).
 *
 * @return  Negative if a < b, zero if a == b, positive if a > b.
 */
PI_Api(int) piStringCompare(const PiString *const a, const PiString *const b);
/**
 * @brief   Lexicographically compare two PiStrings, ignoring case.
 *
 *          Compares characters case-insensitively up to the length of
 *          the shorter string. If the prefixes are equal, the shorter
 *          string is considered less than the longer one.
 *
 * @param[in] a  First string (must not be NULL).
 * @param[in] b  Second string (must not be NULL).
 *
 * @return  Negative if a < b, zero if a == b, positive if a > b.
 */
PI_Api(int) piStringCompareIgnoreCase(const PiString *const a, const PiString *const b);

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
