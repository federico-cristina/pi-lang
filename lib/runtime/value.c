#include "pi/runtime/value.h"

/* =---- Values ------------------------------------------------= */

/**
 * +---- Value::GetHashCode ---------------+
 */

#ifndef PI_HASHCODE_BOOL_TRUE
#   define PI_HASHCODE_BOOL_TRUE 1231
#endif

#ifndef PI_HASHCODE_BOOL_FALSE
#   define PI_HASHCODE_BOOL_FALSE 1237
#endif

static inline uint32_t pi_GetBoolHashCode(const bool value)
{
    return value ? PI_HASHCODE_BOOL_TRUE : PI_HASHCODE_BOOL_FALSE;
}

static inline uint32_t pi_GetCharHashCode(const char value)
{
    return value;
}

#ifndef PI_HashCodeUInt64
#   define PI_HashCodeUInt64(x) \
    ((uint32_t)(x) ^ (uint32_t)((x) >> 32))
#endif

static inline uint32_t pi_GetUIntHashCode(const uint64_t value)
{
    return PI_HashCodeUInt64(value);
}

static inline uint32_t pi_GetSIntHashCode(const int64_t value)
{
    return PI_HashCodeUInt64((uint64_t)value);
}

static inline uint32_t pi_GetRealHashCode(const double value)
{
    const union _pi_DoubleToUInt64
    {
        double   Real;
        uint64_t Long;
    } u = {
        .Real = value
    };

    return PI_HashCodeUInt64(u.Long);
}

uint32_t piValueGetHashCode(const PiValue value)
{
    uint32_t hashCode;

    switch (piValueTypeOf(value))
    {
    case PI_VALUE_TYPE_NONE:
        hashCode = 0;
        break;
    case PI_VALUE_TYPE_BOOL:
        hashCode = pi_GetBoolHashCode(piAsBool(value));
        break;
    case PI_VALUE_TYPE_CHAR:
        hashCode = pi_GetCharHashCode(piAsChar(value));
        break;
    case PI_VALUE_TYPE_UINT:
        hashCode = pi_GetUIntHashCode(piAsUInt(value));
        break;
    case PI_VALUE_TYPE_SINT:
        hashCode = pi_GetSIntHashCode(piAsSInt(value));
        break;
    case PI_VALUE_TYPE_REAL:
        hashCode = pi_GetRealHashCode(piAsReal(value));
        break;
    case PI_VALUE_TYPE_OBJC:
    {
        PiObject *obj = piAsObject(value);

        if (!obj)
        {
            hashCode = 0;
            break;
        }

        switch (obj->type)
        {
        case PI_OBJECT_TYPE_STRING:
        {
            PiStringObject *str = (PiStringObject *)obj;
            hashCode = str->hash; /* Return cached hash */
            break;
        }
        /* Future types: compute hash */
        default:
            /* Pointer hash for other types */
            hashCode = PI_HashCodeUInt64((uintptr_t)obj);
            break;
        }

        break;
    }

    default:
        piUnreachable();
        break;
    }

    return hashCode;
}

/**
 * +---- Value::EqualsTo ------------------+
 */

bool piValueStrictlyEqualsTo(const PiValue lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
    case PI_VALUE_TYPE_BOOL:
    case PI_VALUE_TYPE_CHAR:
    case PI_VALUE_TYPE_UINT:
    case PI_VALUE_TYPE_SINT:
    case PI_VALUE_TYPE_REAL:
        result = (memcmp((const void *)&lValue, (const void *)&rValue, sizeof(PiValue)) == 0);
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline bool pi_BoolEqualsTo(const bool lValue, const PiValue rValue)
{
    return (lValue == piIsTrue(rValue));
}

static inline bool pi_CharEqualsTo(const char lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = false;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = false;
        break;
    case PI_VALUE_TYPE_CHAR:
        result = (lValue == piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = (lValue == (char)piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = (lValue == (char)piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = false;
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline bool pi_UIntEqualsTo(const uint64_t lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = false;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = (lValue == (uint64_t)piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = (lValue == (uint64_t)piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = (lValue == piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = (lValue == (uint64_t)piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = (lValue == (uint64_t)piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline bool pi_SIntEqualsTo(const int64_t lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = false;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = (lValue == (int64_t)piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = (lValue == (int64_t)piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = (lValue == (int64_t)piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = (lValue == piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = (lValue == (int64_t)piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline bool pi_RealEqualsTo(const double lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = false;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = (lValue == (double)piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = false;
        break;
    case PI_VALUE_TYPE_UINT:
        result = (lValue == (double)piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = (lValue == (double)piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = (lValue == piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

bool piValueEqualsTo(const PiValue lValue, const PiValue rValue)
{
    bool result;

    switch (piValueTypeOf(lValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = (piIsNull(lValue) && piIsNull(rValue));
        break;
    case PI_VALUE_TYPE_BOOL:
        result = pi_BoolEqualsTo(piAsBool(lValue), rValue);
        break;
    case PI_VALUE_TYPE_CHAR:
        result = pi_CharEqualsTo(piAsChar(lValue), rValue);
        break;
    case PI_VALUE_TYPE_UINT:
        result = pi_UIntEqualsTo(piAsUInt(lValue), rValue);
        break;
    case PI_VALUE_TYPE_SINT:
        result = pi_SIntEqualsTo(piAsSInt(lValue), rValue);
        break;
    case PI_VALUE_TYPE_REAL:
        result = pi_RealEqualsTo(piAsReal(lValue), rValue);
        break;
    case PI_VALUE_TYPE_OBJC:
    {
        PiObject *lObj = piAsObject(lValue);
        PiObject *rObj = piAsObject(rValue);

        /* Pointer equality first (fast path) */
        if (lObj == rObj)
        {
            result = true;
            break;
        }

        if (!lObj || !rObj)
        {
            result = false;
            break;
        }

        /* Type mismatch check */
        if (!piIsObject(rValue) || lObj->type != rObj->type)
        {
            result = false;
            break;
        }

        /* Type-specific equality */
        switch (lObj->type)
        {
        case PI_OBJECT_TYPE_STRING:
        {
            PiStringObject *lStr = (PiStringObject *)lObj;
            PiStringObject *rStr = (PiStringObject *)rObj;

            /* Length mismatch - early exit */
            if (lStr->length != rStr->length)
            {
                result = false;
                break;
            }

            /* Hash mismatch - early exit */
            if (lStr->hash != rStr->hash)
            {
                result = false;
                break;
            }

            /* Byte-by-byte comparison */
            result = (memcmp(lStr->data, rStr->data, lStr->length) == 0);
            break;
        }
        /* Future types: implement equality */
        default:
            /* Default: pointer equality */
            result = false;
            break;
        }

        break;
    }

    default:
        piUnreachable();
        break;
    }

    return result;
}

/**
 * +---- Value::CompareTo -----------------+
 */

#ifndef PI_CompareBits
#   define PI_CompareBits(x, y) \
    (((x) < (y)) ? -1 : (((x) == (y)) ? 0 : +1))
#endif

static inline int pi_BoolCompareTo(const bool lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = 1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_CompareBits(lValue, piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_CompareBits(lValue, piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_CompareBits(lValue, piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_CompareBits(lValue, piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_CompareBits(lValue, piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline int pi_CharCompareTo(const char lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = 1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_CompareBits(lValue, piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_CompareBits(lValue, piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_CompareBits(lValue, piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_CompareBits(lValue, piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_CompareBits(lValue, piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline int pi_UIntCompareTo(const uint64_t lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = 1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_CompareBits(lValue, piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_CompareBits(lValue, piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_CompareBits(lValue, piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_CompareBits(lValue, piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_CompareBits(lValue, piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline int pi_SIntCompareTo(const int64_t lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = 1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_CompareBits(lValue, piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_CompareBits(lValue, piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_CompareBits(lValue, piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_CompareBits(lValue, piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_CompareBits(lValue, piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

static inline int pi_RealCompareTo(const double lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(rValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = 1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_CompareBits(lValue, piAsBool(rValue));
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_CompareBits(lValue, piAsChar(rValue));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_CompareBits(lValue, piAsUInt(rValue));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_CompareBits(lValue, piAsSInt(rValue));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_CompareBits(lValue, piAsReal(rValue));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

int piValueCompareTo(const PiValue lValue, const PiValue rValue)
{
    int result;

    switch (piValueTypeOf(lValue))
    {
    case PI_VALUE_TYPE_NONE:
        result = (piIsNull(lValue) && piIsNull(rValue)) ? 0 : -1;
        break;
    case PI_VALUE_TYPE_BOOL:
        result = pi_BoolCompareTo(piAsBool(lValue), rValue);
        break;
    case PI_VALUE_TYPE_CHAR:
        result = pi_CharCompareTo(piAsChar(lValue), rValue);
        break;
    case PI_VALUE_TYPE_UINT:
        result = pi_UIntCompareTo(piAsUInt(lValue), rValue);
        break;
    case PI_VALUE_TYPE_SINT:
        result = pi_SIntCompareTo(piAsSInt(lValue), rValue);
        break;
    case PI_VALUE_TYPE_REAL:
        result = pi_RealCompareTo(piAsReal(lValue), rValue);
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

/**
 * +---- Value::ToString ------------------+
 */

#ifndef PI_snprintf
#   ifdef _MSC_VER
#       define PI_snprintf sprintf_s
#   else
#       define PI_snprintf snprintf
#   endif
#endif

int piValueToRawString(char *const buffer, const size_t bufferSize, const PiValue value)
{
    int result;

    switch (piValueTypeOf(value))
    {
    case PI_VALUE_TYPE_NONE:
        result = PI_snprintf(buffer, bufferSize, "null");
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_snprintf(buffer, bufferSize, "%s", piAsBool(value) ? "true" : "false");
        break;
    case PI_VALUE_TYPE_CHAR:
        result = PI_snprintf(buffer, bufferSize, "%c", piAsChar(value));
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_snprintf(buffer, bufferSize, "%" PRIuMAX, (uintmax_t)piAsUInt(value));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_snprintf(buffer, bufferSize, "%" PRIiMAX, (intmax_t)piAsSInt(value));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_snprintf(buffer, bufferSize, "%G", piAsReal(value));
        break;
    case PI_VALUE_TYPE_OBJC:
        piNotImpl();
        break;

    default:
        piUnreachable();
        break;
    }

    return result;
}

/**
 * +---- Value::Print ---------------------+
 */

#ifndef PI_fprintf
#   ifdef _MSC_VER
#       define PI_fprintf fprintf_s
#   else
#       define PI_fprintf fprintf
#   endif
#endif

static const char *pi_GetCharFormat(const int c, const int delim)
{
    const char *format;

    if (!isprint(c) || (c == delim))
    {
        switch (c)
        {
        case '\x07' /* '\a' */:
            format = PI_YELLOW "\\a";
            break;
        case '\x08' /* '\b' */:
            format = PI_YELLOW "\\b";
            break;
        case '\x09' /* '\t' */:
            format = PI_YELLOW "\\t";
            break;
        case '\x0A' /* '\n' */:
            format = PI_YELLOW "\\n";
            break;
        case '\x0B' /* '\v' */:
            format = PI_YELLOW "\\v";
            break;
        case '\x0C' /* '\f' */:
            format = PI_YELLOW "\\f";
            break;
        case '\x0D' /* '\r' */:
            format = PI_YELLOW "\\r";
            break;
        case '\x1B' /* '\e' */:
            format = PI_YELLOW "\\e";
            break;
        case '\x22' /* '\"' */:
            format = PI_YELLOW "\\\"";
            break;
        case '\x27' /* '\'' */:
            format = PI_YELLOW "\\\'";
            break;
        case '\x5C' /* '\\' */:
            format = PI_YELLOW "\\\\";
            break;
        case '\x60' /* '\`' */:
            format = PI_YELLOW "\\`";
            break;

        default:    /* '\xHH' */
            format = PI_YELLOW "\\x%02" PRIX32;
            break;
        }
    }
    else
    {
        format = "%c";
    }

    return format;
}

static inline int pi_PrintFormattedChar(FILE *const stream, const int c, const int delim)
{
    int result = 0;

    result += PI_fprintf(stream, PI_DARK_YELLOW "%c", delim);
    result += PI_fprintf(stream, pi_GetCharFormat(c, delim), c);
    result += PI_fprintf(stream, PI_DARK_YELLOW "%c" PI_RESET, delim);

    return result;
}

int piPrintValueTo(FILE *const stream, const PiValue value)
{
    assert(stream != NULL);

    int result;

    switch (piValueTypeOf(value))
    {
    case PI_VALUE_TYPE_NONE:
        result = PI_fprintf(stream, PI_KeywordColor("null"));
        break;
    case PI_VALUE_TYPE_BOOL:
        result = PI_fprintf(stream, PI_KeywordColor("%s"), piAsBool(value) ? "true" : "false");
        break;
    case PI_VALUE_TYPE_CHAR:
        result = pi_PrintFormattedChar(stream, piAsChar(value), '\'');
        break;
    case PI_VALUE_TYPE_UINT:
        result = PI_fprintf(stream, PI_IntLiteralColor("%" PRIuMAX), (uintmax_t)piAsUInt(value));
        break;
    case PI_VALUE_TYPE_SINT:
        result = PI_fprintf(stream, PI_IntLiteralColor("%" PRIiMAX), (intmax_t)piAsSInt(value));
        break;
    case PI_VALUE_TYPE_REAL:
        result = PI_fprintf(stream, PI_RealLiteralColor("%G"), piAsReal(value));
        break;
    case PI_VALUE_TYPE_OBJC:
    {
        PiObject *obj = piAsObject(value);

        if (!obj)
        {
            result = PI_fprintf(stream, PI_KeywordColor("null"));
            break;
        }

        switch (obj->type)
        {
        case PI_OBJECT_TYPE_STRING:
        {
            PiStringObject *str = (PiStringObject *)obj;
            result = PI_fprintf(stream, PI_DARK_YELLOW "\"" PI_RESET);

            /* Print string with escape sequences */
            for (uint32_t i = 0; i < str->length; i++)
            {
                const char c = str->data[i];
                const char *format = pi_GetCharFormat(c, '"');
                result += PI_fprintf(stream, format, c);
            }

            result += PI_fprintf(stream, PI_DARK_YELLOW "\"" PI_RESET);
            break;
        }
        /* Future types: implement printing */
        default:
            result = PI_fprintf(stream,
                PI_GRAY "<object:%p type=%d>" PI_RESET,
                (void *)obj, obj->type
            );
            break;
        }

        break;
    }

    default:
        piUnreachable();
        break;
    }

    return result;
}

/**
 * +---- ValueArray -----------------------+
 */

PiValueArray *piInitValueArray(PiValueArray *const array, const uint32_t initCap)
{
    assert(array != NULL);

    /* Start with inline buffer to avoid initial heap allocation */
    array->data = array->inline_buffer;
    array->count = 0;
    array->cap = PI_INLINE_STACK_SIZE;

    /* If requested capacity exceeds inline buffer, allocate on heap */
    if (initCap > PI_INLINE_STACK_SIZE)
    {
        array->data = PI_NewArray(PiValue, initCap);
        array->cap = initCap;
    }

    return array;
}

PiValueArray *piFreeValueArray(PiValueArray *const array)
{
    assert(array != NULL);

    /* Only free if heap-allocated (not using inline buffer) */
    if (array->data != array->inline_buffer && array->data != NULL)
        free(array->data);

    array->data = NULL;
    array->count = 0;
    array->cap = 0;

    return array;
}

void piValueArrayResize(PiValueArray *const array, const uint32_t newCap)
{
    assert(array != NULL);

    const uint32_t oldCap = array->cap;

    /* Check if currently using inline buffer */
    if (array->data == array->inline_buffer)
    {
        /* First heap allocation: allocate new buffer and copy from inline */
        array->data = PI_NewArray(PiValue, newCap);

        /* Copy existing values from inline buffer to heap */
        for (uint32_t i = 0; i < array->count && i < oldCap; i++)
            array->data[i] = array->inline_buffer[i];
    }
    else
    {
        /* Regular heap reallocation */
        array->data = PI_Resize(PiValue, array->data, oldCap, newCap);
    }

    if (array->count > newCap)
        array->count = newCap - 1;

    array->cap = newCap;

    return;
}

/* =------------------------------------------------------------= */
