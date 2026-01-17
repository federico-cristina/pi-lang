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

#ifndef _PI_RUNTIME_VALUE_H
#define _PI_RUNTIME_VALUE_H

#include "pi/runtime/object.h"

#include <string.h>
#include <ctype.h>

#if PI_USE_BOXED_VALUES
#   include <math.h>
#endif

#include <stdio.h>

PI_C_HEADER_BEGIN

/* =---- Values ------------------------------------------------= */

#if PI_USE_BOXED_VALUES
#   define UINT48_MAX               ((uint64_t)0x0000FFFFFFFFFFFF)
#   define UINT44_MAX               ((uint64_t)0x00000FFFFFFFFFFF)

/* NaN-boxing generic tags */
#   define PI_TAG_SIGN              ((uint64_t)0x8000000000000000)
#   define PI_TAG_QNAN              ((uint64_t)0x7FFC000000000000)

/* NaN-boxing value tags */
#   define PI_TagType(type)         ((uint64_t)((type) & 0xF) << 44)
#   define PI_TagData(data)         ((uint64_t)(data) & UINT44_MAX)
#   define PI_TagAddr(addr)         ((uint64_t)(addr) & UINT48_MAX)

/* NaN-boxing tags operations */
#   define PI_GetTag(tags, tag)     ((uint64_t)((tags) & (tag)))
#   define PI_SetTag(tags, tag)     ((uint64_t)((tags) | (tag)))
#   define PI_TogTag(tags, tag)     ((uint64_t)((tags) ^ (tag)))

#   define PI_HasTag(tags, tag)     ((bool)(PI_GetTag(tags, tag) == (tag)))
#endif

/**
 * +---- ValueType ------------------------+
 */

/**
 * @brief   Represents the internal value types assumed by the virtual machine's registers and the values
 *          ​​allocated on the stack. However, language variables may assume different types, which are then
 *          mapped to one of these at runtime.
 */
typedef enum _pi_ValueType
{
    /**
     * @brief   None data type (`none`) represents the absence of a type: the only instance of this type is
     *          the value `null`, but it can still be assigned as a type to variables, constants, functions
     *          (especially as a return value, acting as a substitute for the `void` type), and so on.
     * 
     *          Symbols assigned this type are intended to be used to enable metaprogramming, conditional
     *          compilation (or interpretation), and macro construction.
     */
    PI_VALUE_TYPE_NONE = 0x0,
    /**
     * @brief   Boolean data type (`bool`) represents the satisfaction or otherwise of a condition, or a
     *          proposition, which admits only `true` or `false` as possible answers: this type in fact has
     *          only two possible values: `true` and `false` (obviously).
     */
    PI_VALUE_TYPE_BOOL = 0x1,
    /**
     * @brief   Character data type (`char`) represents a text character, therefore the smallest indexable
     *          part of a text string, which is nothing more than a sequence of elements of this type.
     */
    PI_VALUE_TYPE_CHAR = 0x2,
    /**
     * @brief   Unsigned integer data type (or natural, `nat`) represents unsigned integer numbers, the
     *          equivalent of natural numbers in mathematics.
     */
    PI_VALUE_TYPE_UINT = 0x3,
    /**
     * @brief   Signed integer data type (or integer, `int`) represents signed integer numbers, the
     *          equivalent of integer numbers in mathematics.
     */
    PI_VALUE_TYPE_SINT = 0x4,
    /**
     * @brief   Real data type (`real`) represents double-precision floating-point numbers, or `double` in
     *          C, which are the equivalent of real numbers in mathematics.
     * 
     * @note    Despite the existence of integers, this data type is also the default for their representation
     *          (except for special cases where integers are necessarily more efficient). This is because, since
     *          NaN-boxing is adopted as the optimized representation, it is more efficient than unboxing the
     *          integer each time.
     */
    PI_VALUE_TYPE_REAL = 0x5,
    /**
     * @brief   Object data type (`object`) represents the value type that a variable assumes when pointing to
     *          an object of any type in memory.
     */
    PI_VALUE_TYPE_OBJC = 0x6,
} PiValueType;

/**
 * +---- ValueData ------------------------+
 */

/**
 * @brief   Represents the union of the data types that a value can assume.
 */
typedef union _pi_ValueData
{
#if PI_USE_BOXED_VALUES
    uint64_t    TAGS;
#else
    uint8_t    *NONE;
    bool        BOOL;
    char        CHAR;
    uint64_t    UINT;
    int64_t     SINT;
    PiObject   *OBJC;
#endif
    double      REAL;
} PiValueData;

#if PI_USE_BOXED_VALUES
typedef uint32_t pi_uint_t;
typedef  int32_t pi_sint_t;
#else
typedef uint64_t pi_uint_t;
typedef  int64_t pi_sint_t;
#endif

/**
 * @brief   These constants represent the maximum and minimum values ​​that the various numeric types
 *          can assume to ensure correct execution and the absence of arithmetic overflow.
 */
#if PI_USE_BOXED_VALUES
#   define PI_CHAR_MIN  UINT8_C(0)
#   define PI_UINT_MIN UINT32_C(0)
#   define PI_SINT_MIN  INT32_MIN
#   define PI_REAL_MIN    DBL_MIN

#   define PI_CHAR_MAX  UINT8_MAX
#   define PI_UINT_MAX UINT32_MAX
#   define PI_SINT_MAX  INT32_MAX
#   define PI_REAL_MAX    DBL_MAX
#else
#   define PI_CHAR_MIN   CHAR_MIN
#   define PI_UINT_MIN UINT64_C(0)
#   define PI_SINT_MIN  INT64_MIN
#   define PI_REAL_MIN    DBL_MIN

#   define PI_CHAR_MAX   CHAR_MAX
#   define PI_UINT_MAX UINT64_MAX
#   define PI_SINT_MAX  INT64_MAX
#   define PI_REAL_MAX    DBL_MAX
#endif

/**
 * +---- Value ----------------------------+
 */

#if PI_USE_BOXED_VALUES
#   define PI_VALUE_T      \
    PiValueData
#else
#   define PI_VALUE_T      \
    struct _pi_Value       \
    {                      \
        PiValueType type;  \
        PiValueData as;    \
    }
#endif

/**
 * @brief   Represents the data type that the values ​​will assume within the virtual machine during
 *          operations.
 */
typedef PI_VALUE_T PiValue;

#if PI_USE_BOXED_VALUES
#   define PI_GetValue(type, data)                      \
    {                                                   \
        .TAGS = PI_TAG_QNAN                             \
              | PI_TagType(PI_VALUE_TYPE_ ## type)      \
              | PI_TagData(data)                        \
    }
#   define PI_GetValueType(value)                       \
    PI_GetTag((value).TAGS >> 44, 0xF)
#   define PI_GetValueData(value)                       \
    PI_GetTag((value).TAGS, UINT44_MAX)
#   define PI_GetValueAddr(value)                       \
    PI_TogTag((value).TAGS, (PI_TAG_SIGN | PI_TAG_QNAN))
#else
#   define PI_GetValue(type_, data)                     \
    {                                                   \
        .type = PI_VALUE_TYPE_ ## type_,                \
        .as = {                                         \
            .type_ = data                               \
        }                                               \
    }
#   define PI_GetValueType(value)                       \
    ((value).type)
#   define PI_GetValueData(value)                       \
    ((value).as.UINT)
#   define PI_GetValueAddr(value)                       \
    ((uintptr_t)(value).as.NONE)
#endif

/**
 * @brief   This constant represents `null` value.
 */
static const PiValue PI_NULL = PI_GetValue(NONE, NULL);
/**
 * @brief   This constant represents `true` value.
 */
static const PiValue PI_TRUE = PI_GetValue(BOOL, true);
/**
 * @brief   This constant represents `false` value.
 */
static const PiValue PI_FALSE = PI_GetValue(BOOL, false);

/**
 * +---- Value::From ----------------------+
 */

/**
 * @brief   This function takes as input a value of type `bool` and returns it boxed in a `PiValue`. 
 */
static inline PiValue piBool(const bool value)
{
    return value ? PI_TRUE : PI_FALSE;
}
/**
 * @brief   This function takes as input a value of type `char` and returns it boxed in a `PiValue`. 
 */
static inline PiValue piChar(const char value)
{
    return (PiValue)PI_GetValue(CHAR, value);
}
/**
 * @brief   This function takes as input a value of type `pi_uint_t` and returns it boxed in a `PiValue`. 
 */
static inline PiValue piUInt(const pi_uint_t value)
{
    return (PiValue)PI_GetValue(UINT, value);
}
/**
 * @brief   This function takes as input a value of type `pi_sint_t` and returns it boxed in a `PiValue`.  
 */
static inline PiValue piSInt(const pi_sint_t value)
{
    return (PiValue)PI_GetValue(SINT, value);
}
/**
 * @brief   This function takes as input a value of type `double` and returns it boxed in a `PiValue`. 
 */
static inline PiValue piReal(const double value)
{
#if PI_USE_BOXED_VALUES
    if (isnan(value))
        return (PiValue) {
            .REAL = NAN
        };

    if (isinf(value))
        return (PiValue) {
            .REAL = INFINITY
        };

    return (PiValue) {
        .REAL = value
    };
#else
    return (PiValue)PI_GetValue(REAL, value);
#endif
}
/**
 * @brief   This function takes as input a value of type `PiObject` and returns it boxed in a `PiValue`. 
 */
static inline PiValue piObject(PiObject *const value)
{
    if (!value)
        return PI_NULL;

#if PI_USE_BOXED_VALUES
    return (PiValue) {
        .TAGS = PI_TAG_SIGN | PI_TAG_QNAN | (uintptr_t)value
    };
#else
    return (PiValue)PI_GetValue(OBJC, value);
#endif
}

/**
 * +---- Value::Is ------------------------+
 */

/**
 * @brief   This function checks if the value type of the specified value is `none`.
 */
static inline bool piIsNone(const PiValue value)
{
    return PI_GetValueType(value) == PI_VALUE_TYPE_NONE;
}
/**
 * @brief   This function checks if the value type of the specified value is `bool`.
 */
static inline bool piIsBool(const PiValue value)
{
    return PI_GetValueType(value) == PI_VALUE_TYPE_BOOL;
}
/**
 * @brief   This function checks if the value type of the specified value is `char`.
 */
static inline bool piIsChar(const PiValue value)
{
    return PI_GetValueType(value) == PI_VALUE_TYPE_CHAR;
}
/**
 * @brief   This function checks if the value type of the specified value is `nat`.
 */
static inline bool piIsUInt(const PiValue value)
{
    return PI_GetValueType(value) == PI_VALUE_TYPE_UINT;
}
/**
 * @brief   This function checks if the value type of the specified value is `int`.
 */
static inline bool piIsSInt(const PiValue value)
{
    return PI_GetValueType(value) == PI_VALUE_TYPE_SINT;
}
/**
 * @brief   This function checks if the value type of the specified value is `real`.
 */
static inline bool piIsReal(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return !PI_HasTag((value).TAGS, PI_TAG_QNAN);
#else
    return PI_GetValueType(value) == PI_VALUE_TYPE_REAL;
#endif
}
/**
 * @brief   This function checks if the value type of the specified value is `object`.
 */
static inline bool piIsObject(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return PI_HasTag((value).TAGS, PI_TAG_SIGN | PI_TAG_QNAN);
#else
    return PI_GetValueType(value) == PI_VALUE_TYPE_OBJC;
#endif
}

/**
 * +---- Value::TypeOf --------------------+
 */

/**
 * @brief   This function returns the value type of the specified value.
 */
static inline PiValueType piValueTypeOf(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    if (piIsReal(value))
        return PI_VALUE_TYPE_REAL;

    if (piIsObject(value))
        return PI_VALUE_TYPE_OBJC;
#endif

    return PI_GetValueType(value);
}

/**
 * +---- Value::As ------------------------+
 */

/**
 * @brief   This function gets the `bool` representation of this value.
 */
static inline bool piAsBool(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return (bool)PI_GetValueData(value);
#else
    return value.as.BOOL;
#endif
}
/**
 * @brief   This function gets the `char` representation of this value.
 */
static inline char piAsChar(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return (char)PI_GetValueData(value);
#else
    return value.as.CHAR;
#endif
}
/**
 * @brief   This function gets the `nat` representation of this value.
 */
static inline pi_uint_t piAsUInt(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return (pi_uint_t)PI_GetValueData(value);
#else
    return value.as.UINT;
#endif
}
/**
 * @brief   This function gets the `int` representation of this value.
 */
static inline pi_sint_t piAsSInt(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return (pi_sint_t)PI_GetValueData(value);
#else
    return value.as.SINT;
#endif
}
/**
 * @brief   This function gets the `real` representation of this value.
 */
static inline double piAsReal(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return value.REAL;
#else
    return value.as.REAL;
#endif
}
/**
 * @brief   This function gets the reference to the object represented by this value.
 */
static inline PiObject *piAsObject(const PiValue value)
{
#if PI_USE_BOXED_VALUES
    return (PiObject *)PI_GetValueAddr(value);
#else
    return value.as.OBJC;
#endif
}

/**
 * +---- Value::IsNull/IsTrue -------------+
 */

/**
 * @brief   This function checks if the value is `null`.
 */
static inline bool piIsNull(const PiValue value)
{
    return memcmp(&value, &PI_NULL, sizeof(PiValue)) == 0;
}
/**
 * @brief   This function checks if the value is `true`.
 */
static inline bool piIsTrue(const PiValue value)
{
#if PI_DEBUG
    bool result;

    switch (piValueTypeOf(value))
    {
    case PI_VALUE_TYPE_NONE:
        result = false;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = piAsBool(value);
        break;
    case PI_VALUE_TYPE_CHAR:
        result = piAsChar(value) != '\0';
        break;
    case PI_VALUE_TYPE_UINT:
        result = piAsUInt(value) != 0U;
        break;
    case PI_VALUE_TYPE_SINT:
        result = piAsSInt(value) != 0;
        break;
    case PI_VALUE_TYPE_REAL:
        result = piAsReal(value) != 0.;
        break;
    case PI_VALUE_TYPE_OBJC:
        result = piAsObject(value) != NULL;
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
#else
    return PI_GetValueData(value) != 0;
#endif
}

/**
 * +---- Value::GetHashCode ---------------+
 */

/**
 * @brief   This function computes the hash code of the specified value.
 */
uint32_t piValueGetHashCode(const PiValue value);

/**
 * +---- Value::EqualsTo ------------------+
 */

/**
 * @brief   This function returns true when the two specified values are the same.
 */
bool piValueStrictlyEqualsTo(const PiValue lValue, const PiValue rValue);
/**
 * @brief   This function returns true when the two specified values equals.
 */
bool piValueEqualsTo(const PiValue lValue, const PiValue rValue);

/**
 * +---- Value::CompareTo -----------------+
 */

/**
 * @brief   This function compares two values and returns a value: > 0 when the first value
 *          preceeds the second one, = 0 when they are equal and, finally, < 0 when the first
 *          value is preceeded by the second one.
 */
int piValueCompareTo(const PiValue lValue, const PiValue rValue);

/**
 * +---- Value::ToString ------------------+
 */

int piValueToRawString(char *const buffer, const size_t bufferSize, const PiValue value);

/**
 * +---- Value::Print ---------------------+
 */

/**
 * @brief   This function prints, on the specified stream, a human-readable representation of the
 *          specified value.
 */
int piPrintValueTo(FILE *const stream, const PiValue value);
/**
 * @brief   This function prints, on the standard output stream, a human-readable representation
 *          of the specified value.
 */
static inline int piPrintValue(const PiValue value)
{
    return piPrintValueTo(stdout, value);
}

/**
 * +---------------------------------------+
 */

#undef PI_VALUE_T

#undef PI_GetValue
#undef PI_GetValueType
#undef PI_GetValueData
#undef PI_GetValueAddr

/**
 * +---- ValueArray -----------------------+
 */

#ifndef PI_DEFAULT_ARRAY_CAP
#   define PI_DEFAULT_ARRAY_CAP (64 / sizeof(PiValue))
#endif

typedef struct _pi_ValueArray
{
    PiValue *data;
    uint32_t count;
    uint32_t cap;
} PiValueArray;

PiValueArray *piInitValueArray(PiValueArray *const array, const uint32_t initCap);
PiValueArray *piFreeValueArray(PiValueArray *const array);

void piValueArrayResize(PiValueArray *const array, const uint32_t newCap);

#ifndef PI_ValueArrayGrow
#   define PI_ValueArrayGrow(array) \
    piValueArrayResize((array), (array)->cap ? (uint32_t)(size_t)((array)->cap * 2) : PI_DEFAULT_ARRAY_CAP)
#endif

static inline uint32_t piValueArrayPush(PiValueArray *const array, const PiValue value)
{
    assert(array != NULL);

    if (PI_ShouldGrow(array->cap, array->count, 1))
        PI_ValueArrayGrow(array);

    array->data[array->count] = value;

    /* Returns the index of the new item */
    return array->count++;
}

#ifndef PI_ValueArrayIsEmpty
#   define PI_ValueArrayIsEmpty(array) \
    ((array)->count == 0)
#endif

static inline PiValue piValueArrayTop(const PiValueArray *const array)
{
    assert(array != NULL);

    if (PI_ValueArrayIsEmpty(array))
        piRaiseError("stack underflow", NULL);

    return array->data[array->count - 1];
}

static inline PiValue piValueArrayPop(PiValueArray *const array)
{
    assert(array != NULL);

    if (PI_ValueArrayIsEmpty(array))
        piRaiseError("stack underflow", NULL);

    return array->data[--array->count];
}

static inline PiValue piValueArrayGet(PiValueArray *const array, const uint32_t index)
{
    assert(array != NULL);

    if (PI_ValueArrayIsEmpty(array))
        piRaiseError("index out of bounds (the array is empty)", NULL);

    return array->data[index];
}

/* =------------------------------------------------------------= */

PI_C_HEADER_END

#endif
