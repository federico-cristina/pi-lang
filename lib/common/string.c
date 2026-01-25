#include "pi/common/string.h"

#include <string.h>
#include <ctype.h>

/* =---- FNV-1a Hash Function ----------------------------------= */

PI_Api(uint32_t) piHashString(const char *const data, const uint32_t length)
{
    assert(data != NULL);

    uint32_t hash = PI_FNV_BASIS;

    for (uint32_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)data[i];
        hash *= PI_FNV_PRIME;
    }

    return hash;
}

/* =---- String ------------------------------------------------= */

static inline PiString *pi_InitString(PiString *const str, const char *const cstr, const uint32_t length, const bool clear, const uint32_t hash)
{
    assert(str != NULL);

    str->chars = cstr;
    str->count = length;
    str->clear = clear;
    str->hash = hash;

    return str;
}

PI_Api(PiString *) piInitString(PiString *const str, const char *const cstr)
{
    assert(cstr != NULL);

    const uint32_t length = (uint32_t)strlen(cstr);

    return pi_InitString(str, cstr, length, false, piHashString(cstr, length));
}

PI_Api(PiString *) piInitStringWithLength(PiString *const str, const char *const cstr, const uint32_t length)
{
    assert(cstr != NULL); 

    return pi_InitString(str, cstr, length, false, piHashString(cstr, length));
}

PI_Api(PiString *) piInitOwnedString(PiString *const str, char *const cstr)
{
    assert(cstr != NULL);

    const uint32_t length = (uint32_t)strlen(cstr);

    return pi_InitString(str, cstr, length, true, piHashString(cstr, length));
}

PI_Api(PiString *) piInitOwnedStringWithLenght(PiString *const str, char *const cstr, const uint32_t length)
{
    assert(cstr != NULL); 

    return pi_InitString(str, cstr, length, true, piHashString(cstr, length));
}

/**
 * +---- String::Copy ---------------------+
 */

static inline PiString *pi_StringCopyTo(const PiString *const dest, const PiString *const str)
{
    assert(str != NULL);

    PiString *const result = dest ? dest : PI_New(PiString);

    result->chars = str->chars;
    result->count = str->count;
    result->clear = str->clear;
    result->hash = str->hash;

    return result;
}

PI_Api(PiString *) piStringCopy(const PiString *const str)
{
    return pi_StringCopyTo(NULL, str);
}

PI_Api(PiString *) piStringCopyTo(const PiString *const dest, const PiString *const str)
{
    return pi_StringCopyTo(dest, str);
}

/**
 * +---- String::Dup ----------------------+
 */

static inline PiString *pi_StringDupTo(const PiString *const dest, const PiString *const str)
{
    assert(str != NULL);

    PiString *const result = dest ? dest : PI_New(PiString);

    if (str->chars)
        result->chars = (const char *)strncpy(PI_NewArray(char, str->count + 1), str->chars, str->count + 1);
    else
        result->chars = NULL;

    result->count = str->count;
    result->clear = true;
    result->hash = str->hash;

    return result;
}

PI_Api(PiString *) piStringDup(const PiString *const str)
{
    return pi_StringDupTo(NULL, str);
}

PI_Api(PiString *) piStringDupTo(const PiString *const dest, const PiString *const str)
{
    return pi_StringDupTo(dest, str);
}

/**
 * +---- String::Free ---------------------+
 */

PI_Api(PiString *) piFreeString(PiString *const str)
{
    if (str == NULL)
        return str;

    if (str->clear && str->chars)
        piFree((void *)str->chars);
    
    str->chars = NULL;
    str->count = 0;
    str->clear = 0;
    str->hash  = 0;

    return str;
}

/**
 * +---- String::Equals -------------------+
 */

PI_Api(bool) piStringEquals(const PiString *const a, const PiString *const b)
{
    assert(a != NULL);
    assert(b != NULL);

    /* Compare hash first */
    if (a->hash != b->hash)
        return false;

    /* Compare length */
    if (a->count != b->count)
        return false;

    /* Compare actual characters */
    return memcmp(a->chars, b->chars, a->count) == 0;
}

PI_Api(bool) piStringEqualsIgnoreCase(const PiString *const a, const PiString *const b)
{
    assert(a != NULL);
    assert(b != NULL);

    /* Compare length first (hash is case-sensitive, so skip it) */
    if (a->count != b->count)
        return false;

    /* Compare characters case-insensitively */
    for (uint32_t i = 0; i < a->count; i++)
    {
        if (tolower((unsigned char)a->chars[i]) != tolower((unsigned char)b->chars[i]))
            return false;
    }

    return true;
}

/**
 * +---- String::Compare ------------------+
 */

PI_Api(int) piStringCompare(const PiString *const a, const PiString *const b)
{
    assert(a != NULL);
    assert(b != NULL);

    /* Compare using the shorter length */
    const uint32_t minLen =
        (a->count < b->count) ? a->count : b->count;
    /* Compare string bytes */
    const int cmp = memcmp(a->chars, b->chars, minLen);

    if (cmp != 0)
        return cmp;

    /* If prefixes are equal, shorter string comes first */
    if (a->count < b->count)
        return -1;

    if (a->count > b->count)
        return 1;

    return 0;
}

PI_Api(int) piStringCompareIgnoreCase(const PiString *const a, const PiString *const b)
{
    assert(a != NULL);
    assert(b != NULL);

    /* Compare characters case-insensitively up to shorter length */
    const uint32_t minLen =
        (a->count < b->count) ? a->count : b->count;

    for (uint32_t i = 0; i < minLen; i++)
    {
        const int ca = tolower((unsigned char)a->chars[i]);
        const int cb = tolower((unsigned char)b->chars[i]);

        if (ca != cb)
            return ca - cb;
    }

    /* If prefixes are equal, shorter string comes first */
    if (a->count < b->count)
        return -1;

    if (a->count > b->count)
        return 1;

    return 0;
}

/* =------------------------------------------------------------= */
