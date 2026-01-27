#ifndef _PI_COMPILER_DIAGNOSTIC_H
#   error This file should not be included directly. Include diagnostic.h instead.
#endif

/* =---- Errors Codes Definition -------------------------------= */

#ifndef PI_DefineError
#   define PI_DefineError(name, code, level, kind, format, ...)
#endif

PI_DefineError(DEFAULT,                         0x0001, ERROR, ERROR, "%s")

/**
 * +---- Lexical Errors -------------------+
 */

#ifndef PI_DefineSyntaxError
#   define PI_DefineSyntaxError(name, code, level, format, ...) \
    PI_DefineError(name, code, level, SYNTAX_ERROR, format)
#endif

PI_DefineSyntaxError(EXPECTED_SEMICOLON,        0xC001, ERROR, "expected ';' after expression")

#undef PI_DefineSyntaxError

/**
 * +---------------------------------------+
 */

#undef PI_DefineError

/* =------------------------------------------------------------= */

