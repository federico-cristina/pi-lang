#include "pi/runtime/string.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

/* =---- Internal Functions ------------------------------------= */

#define FNV_BASIS 2166136261u
#define FNV_PRIME 16777619u

/**
 * @brief   Compute FNV-1a hash for string data.
 *
 * FNV-1a (Fowler-Noll-Vo hash function, variant 1a) is a fast,
 * non-cryptographic hash function with good distribution properties.
 *
 * @param[in] data   String data to hash.
 * @param[in] length Length of data in bytes.
 *
 * @return 32-bit hash code.
 */
static uint32_t pi_HashString(const char *const data, const uint32_t length)
{
    assert(data != NULL);

    /* FNV-1a constants for 32-bit hash */
    uint32_t hash = FNV_BASIS;

    for (uint32_t i = 0; i < length; i++)
    {
        hash ^= (uint8_t)data[i];
        hash *= FNV_PRIME;
    }

    return hash;
}

/* =---- Public API --------------------------------------------= */

/**
 * +---- String Allocation ----------------+
 */

PI_Api(PiStringObject *) piStringNew(PiEnv *const env, const char *const cstr, const uint32_t length)
{
    assert(env != NULL);
    assert(cstr != NULL);

    /* Compute the size of the object to allocate */
    const size_t size = sizeof(PiStringObject) + length + 1;
    /* Allocate the managed string object */
    PiStringObject *const str = (PiStringObject *)piGcAlloc(env, PI_OBJECT_TYPE_STRING, size);

    /* Initialize string-specific fields */
    str->length = length;
    str->hash = pi_HashString(cstr, length);

    /* Copy string data */
    memcpy(str->data, cstr, length);

    str->data[length] = '\0';

    return str;
}

PI_Api(PiStringObject *) piStringFormat(PiEnv *const env, const char *const format, ...)
{
    assert(env != NULL);
    assert(format != NULL);

    va_list args;

    /* Calculate required buffer size */
    va_start(args, format);

    const int length = vsnprintf(NULL, 0, format, args);

    va_end(args);

    if (length < 0)
        piRaiseError("string format error: %s", (void *)format);

    /* Compute the size of the object to allocate */
    const size_t size = sizeof(PiStringObject) + length + 1;
    /* Allocate the managed string object */
    PiStringObject *const str = (PiStringObject *)piGcAlloc(env, PI_OBJECT_TYPE_STRING, size);

    str->length = (uint32_t)length;

    /* Format string into data buffer */
    va_start(args, format);
#ifdef _MSC_VER
    vsnprintf_s(str->data, (size_t)length + 1, (size_t)length + 1, format, args);
#else
    vsnprintf(str->data, (size_t)length + 1, format, args);
#endif
    va_end(args);

    str->hash = pi_HashString(str->data, str->length);

    return str;
}

/* =------------------------------------------------------------= */
