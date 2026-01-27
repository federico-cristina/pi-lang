#ifndef _PI_COMPILER_TOKEN_H
#   error This file should not be included directly. Include token.h instead.
#endif

/* =---- Lexical Token Codes Definition ------------------------= */

#ifndef PI_DefineToken
#   define PI_DefineToken(name, ...)
#endif

/**
 * +---- Trivial --------------------------+
 */

PI_DefineToken(ENDOF)
PI_DefineToken(ENDOL)

PI_DefineToken(COMMENT)

/**
 * +---- Keyword and Identifier -----------+
 */

PI_DefineToken(IDENT)

#ifndef PI_DefineKeywordToken
#   define PI_DefineKeywordToken(name, text, color, ...) \
    PI_DefineToken(name, __VA_ARGS__)
#endif

PI_DefineKeywordToken(KWORD_VAR,    "var",      PI_BLUE)

#undef PI_DefineKeywordToken

/**
 * +---- Brackets -------------------------+
 */

PI_DefineToken(BRACK_ROUND_OPEN,        "(")
PI_DefineToken(BRACK_ROUND_CLOSE,       ")")

PI_DefineToken(BRACK_SQUARED_OPEN,      "[")
PI_DefineToken(BRACK_SQUARED_CLOSE,     "]")

PI_DefineToken(BRACK_CURLY_OPEN,        "{")
PI_DefineToken(BRACK_CURLY_CLOSE,       "}")

PI_DefineToken(BRACK_ANGLE_OPEN,        "<")
PI_DefineToken(BRACK_ANGLE_CLOSE,       ">")

/**
 * +---- Punctuators ----------------------+
 */

PI_DefineToken(PUNCT_TILDE,             "~")
PI_DefineToken(PUNCT_TILDE_EQUAL,       "~=")

PI_DefineToken(PUNCT_EXCLM,             "!")
PI_DefineToken(PUNCT_EXCLM_EXCLM,       "!!")
PI_DefineToken(PUNCT_EXCLM_EQUAL,       "!=")

PI_DefineToken(PUNCT_QUEST,             "?")
PI_DefineToken(PUNCT_QUEST_QUEST,       "??")
PI_DefineToken(PUNCT_QUEST_EQUAL,       "?=")
PI_DefineToken(PUNCT_QUEST_DOT,         "?.")

PI_DefineToken(PUNCT_AMPER,             "&")
PI_DefineToken(PUNCT_AMPER_AMPER,       "&&")
PI_DefineToken(PUNCT_AMPER_EQUAL,       "&=")

PI_DefineToken(PUNCT_PIPE,              "|")
PI_DefineToken(PUNCT_PIPE_PIPE,         "||")
PI_DefineToken(PUNCT_PIPE_EQUAL,        "|=")

PI_DefineToken(PUNCT_CARET,             "^")
PI_DefineToken(PUNCT_CARET_EQUAL,       "^=")

PI_DefineToken(PUNCT_EQUAL,             "=")
PI_DefineToken(PUNCT_EQUAL_EQUAL,       "==")
PI_DefineToken(PUNCT_THICK_ARROW,       "=>")

PI_DefineToken(PUNCT_LESS,              "<")
PI_DefineToken(PUNCT_LESS_LESS,         "<<")
PI_DefineToken(PUNCT_LESS_LESS_EQUAL,   "<<=")
PI_DefineToken(PUNCT_LESS_EQUAL,        "<=")
PI_DefineToken(PUNCT_LEFT_ARROW,        "<-")

PI_DefineToken(PUNCT_GREAT,             ">")
PI_DefineToken(PUNCT_GREAT_GREAT,       ">>")
PI_DefineToken(PUNCT_GREAT_GREAT_EQUAL, ">>=")
PI_DefineToken(PUNCT_GREAT_EQUAL,       ">=")

PI_DefineToken(PUNCT_PLUS,              "+")
PI_DefineToken(PUNCT_PLUS_PLUS,         "++")
PI_DefineToken(PUNCT_PLUS_EQUAL,        "+=")

PI_DefineToken(PUNCT_MINUS,             "-")
PI_DefineToken(PUNCT_MINUS_MINUS,       "--")
PI_DefineToken(PUNCT_MINUS_EQUAL,       "-=")
PI_DefineToken(PUNCT_ARROW,             "->")

PI_DefineToken(PUNCT_STAR,              "*")
PI_DefineToken(PUNCT_STAR_STAR,         "**")
PI_DefineToken(PUNCT_STAR_EQUAL,        "*=")

PI_DefineToken(PUNCT_SLASH,             "/")
PI_DefineToken(PUNCT_SLASH_EQUAL,       "/=")

PI_DefineToken(PUNCT_PERC,              "%")
PI_DefineToken(PUNCT_PERC_EQUAL,        "%=")

PI_DefineToken(PUNCT_DOT,               ".")
PI_DefineToken(PUNCT_DOT_DOT,           "..")
PI_DefineToken(PUNCT_DOT_DOT_DOT,       "...")

PI_DefineToken(PUNCT_COLON,             ":")
PI_DefineToken(PUNCT_COLON_COLON,       "::")
PI_DefineToken(PUNCT_COLON_EQUAL,       ":=")

PI_DefineToken(PUNCT_SEMIC,             ";")
PI_DefineToken(PUNCT_SEMIC_SEMIC,       ";;")

PI_DefineToken(PUNCT_COMMA,             ",")

PI_DefineToken(PUNCT_HASH,              "#")
PI_DefineToken(PUNCT_HASH_HASH,         "##")

PI_DefineToken(PUNCT_AT,                "@")

/**
 * +---- Literals -------------------------+
 */

PI_DefineToken(LITER_INT)
PI_DefineToken(LITER_REAL)

/**
 * +---------------------------------------+
 */

#undef PI_DefineToken

/* =------------------------------------------------------------= */
