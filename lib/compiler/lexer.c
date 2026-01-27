/**
 * @file        lexer.c
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
 * @brief       Lexical analyzer implementation with token scanning.
 */

#include "pi/compiler/lexer.h"

#include <string.h>
#include <ctype.h>

/* =---- Lexical Analyzer --------------------------------------= */

static bool pi_IsIdentStart(const int c)
{
    return isalpha(c) || (c == '_') || (c == '$');
}

static bool pi_IsIdent(const int c)
{
    return isalnum(c) || (c == '_');
}

static bool pi_IsBinaryDigit(const int c)
{
    return (c == '0') || (c == '1') || (c == '_');
}

static bool pi_IsOctalDigit(const int c)
{
    return ((c >= '0') && (c <= '7')) || (c == '_');
}

static bool pi_IsDecimalDigit(const int c)
{
    return ((c >= '0') && (c <= '9')) || (c == '_');
}

static bool pi_IsHexadecimalDigit(const int c)
{
    return ((c >= '0') && (c <= '9')) || ((toupper(c) >= 'A') && (toupper(c) <= 'F')) || (c == '_');
}

/* =---- Lexical Analysis --------------------------------------= */

bool piGetKeyword(PiKeyword *const keyword, const char *const start, const uint32_t length)
{
    if (length <= 1)
        return false;

    bool result = false;

    static const PiKeyword pi_KeywordTable[] =
    {
#ifndef PI_DefineKeywordToken
#   ifdef PI_USE_COLORS
#       define PI_DefineKeywordToken(name, _text, _color, ...)  \
        {                                                       \
            .color = _color,                                    \
            .text = _text,                                      \
            .length = PI_CountOf(_text),                        \
            .code = PI_TOKEN_ ## name,                          \
        },
#   else
#       define PI_DefineKeywordToken(name, _text, ...)          \
        {                                                       \
            .text = _text,                                      \
            .length = PI_CountOf(_text),                        \
            .code = PI_TOKEN_ ## name,                          \
        },
#   endif
#endif

#include "pi/compiler/token.inc"

        {
#ifdef PI_USE_COLORS
            .color = "",
#endif
            .text = NULL,
            .length = 0,
            .code = PI_TOKEN_INVAL,
        }
    };

    register uint32_t low = 0, high = PI_CountOf(pi_KeywordTable) - 1, mid;

    do
    {
        mid = (low + high) / 2;

        register const int
            cmp = strncmp(start, pi_KeywordTable[mid].text, length);

        if (!cmp)
        {
            if (keyword)
                *keyword = pi_KeywordTable[mid];

            result = true;
        }
        else
        {
            if (cmp > 0)
                high = mid + 1;
            else
                low = mid + 1;
        }
    } while (!result && ((high - low) > 1));
    
    return result;
}

/**
 * +---- Token ----------------------------+
 */

static PiTokenCode pi_MakeNumericToken(PiToken *const token, const PiTokenCode code, const PiSource *const source, const int radix)
{
    if (token)
    {
        token->source = source->name;

        const uint32_t length = piSourceGetLexemeLength(source);

        token->text = piSourceGetRawLexeme(source);
        token->length = length;

        token->line = source->currentPos.line;
        token->column = source->currentPos.column;

        token->code = code;
        token->radix = radix;
    }

    return code;
}

static inline PiTokenCode pi_MakeToken(PiToken *const token, const PiTokenCode code, const PiSource *const source)
{
    return pi_MakeNumericToken(token, code, source, /* radix: */ 0);
}

static inline PiTokenCode pi_MakeEndOfFileToken(PiToken *const token, const PiSource *const source)
{
    return pi_MakeNumericToken(token, PI_TOKEN_ENDOF, source, /* radix: */ 0);
}

/**
 * +---- Scanner --------------------------+
 */

static inline PiTokenCode pi_ScanNumericLiteralWithRadix(PiSource *const source, PiToken *const outToken, const PiPredicateFn predicate, const int radix)
{
    PiTokenCode result;

    if (piSourceCheckPred(source, predicate))
    {
        do
            piSourceRead(source);
        while (piSourceCheckPred(source, predicate));

        result = pi_MakeNumericToken(outToken, PI_TOKEN_LITER_INT, source, radix);
    }
    else
    {
        result = PI_TOKEN_INVAL;
    }

    return result;
}

static inline PiTokenCode pi_ScanBinaryLiteral(PiSource *const source, PiToken *const outToken)
{
    return pi_ScanNumericLiteralWithRadix(source, outToken, pi_IsBinaryDigit, /* radix: */ 2);
}

static inline PiTokenCode pi_ScanOctalLiteral(PiSource *const source, PiToken *const outToken)
{
    return pi_ScanNumericLiteralWithRadix(source, outToken, pi_IsOctalDigit, /* radix: */ 8);
}

static inline PiTokenCode pi_ScanDecimalLiteral(PiSource *const source, PiToken *const outToken)
{
    return pi_ScanNumericLiteralWithRadix(source, outToken, pi_IsDecimalDigit, /* radix: */ 10);
}

static inline PiTokenCode pi_ScanHexadecimalLiteral(PiSource *const source, PiToken *const outToken)
{
    return pi_ScanNumericLiteralWithRadix(source, outToken, pi_IsHexadecimalDigit, /* radix: */ 16);
}

static PiTokenCode pi_ScanNumericLiteral(PiSource *const source, PiToken *const outToken)
{
    PiTokenCode result;

    if (piSourceMatch(source, '0'))
    {
        const int c = tolower(piSourceRead(source));

        switch (c)
        {
        case 'b':
            result = pi_ScanBinaryLiteral(source, outToken);
            break;
        case 'c':
            result = pi_ScanOctalLiteral(source, outToken);
            break;
        case 'd':
            result = pi_ScanDecimalLiteral(source, outToken);
            break;
        case 'x':
            result = pi_ScanHexadecimalLiteral(source, outToken);
            break;

        case 't':
            /* ToDo: Hypotetical temporal base */
            break;

        default:
            break;
        }
    }
    else
    {
        result = pi_ScanDecimalLiteral(source, outToken);
    }

    return result;
}

static PiTokenCode pi_ScanIdentifierOrKeyword(PiSource *const source, PiToken *const outToken)
{
    PiTokenCode result;

    do
        piSourceRead(source);
    while (piSourceCheckPred(source, pi_IsIdent));

    PiKeyword keyword;

    if (piGetKeyword(&keyword, piSourceGetRawLexeme(source), piSourceGetLexemeLength(source)))
        result = keyword.code;
    else
        result = PI_TOKEN_IDENT;

    return pi_MakeToken(outToken, result, source);
}

static inline PiTokenCode pi_ScanSingleLineComment(PiSource *const source, PiToken *const outToken)
{
    int c;

    do
        c = piSourceRead(source);
    while ((c != '\n') && (c != EOF));

    return PI_TOKEN_COMMENT;
}

static inline PiTokenCode pi_ScanMultiLineComment(PiSource *const source, PiToken *const outToken)
{
    int c, counter = 1;

    do
    {
        c = piSourceRead(source);

        if ((c == '*') && piSourceMatch(source, '/'))
            --counter;
        else if ((c == '/') && piSourceMatch(source, '*'))
            ++counter;
        else
            /* Nothing! */;
    } while ((counter > 0) && (c != EOF));

    return PI_TOKEN_COMMENT;
}

static PiTokenCode pi_ScanPunctuator(PiSource *const source, PiToken *const outToken)
{
    int c, result;

    switch (c = piSourceRead(source))
    {
        /**
         * +---- Brackets -------------------------+
         */

    case '(':
        result = PI_TOKEN_BRACK_ROUND_OPEN;
        break;
    case ')':
        result = PI_TOKEN_BRACK_ROUND_CLOSE;
        break;

    case '[':
        result = PI_TOKEN_BRACK_ROUND_OPEN;
        break;
    case ']':
        result = PI_TOKEN_BRACK_ROUND_CLOSE;
        break;

    case '{':
        result = PI_TOKEN_BRACK_ROUND_OPEN;
        break;
    case '}':
        result = PI_TOKEN_BRACK_ROUND_CLOSE;
        break;

        /**
         * +---- Punctuators ----------------------+
         */

    case '~':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_TILDE_EQUAL;
        else
            result = PI_TOKEN_PUNCT_TILDE;
        break;

    case '!':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_EXCLM_EQUAL;
        else if (piSourceMatch(source, '!'))
            result = PI_TOKEN_PUNCT_EXCLM_EXCLM;
        else
            result = PI_TOKEN_PUNCT_EXCLM;
        break;
    case '?':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_QUEST_EQUAL;
        else if (piSourceMatch(source, '?'))
            result = PI_TOKEN_PUNCT_QUEST_QUEST;
        else if (piSourceMatch(source, '.'))
            result = PI_TOKEN_PUNCT_QUEST_DOT;
        else
            result = PI_TOKEN_PUNCT_QUEST;
        break;

    case '&':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_AMPER_EQUAL;
        else if (piSourceMatch(source, '&'))
            result = PI_TOKEN_PUNCT_AMPER_AMPER;
        else
            result = PI_TOKEN_PUNCT_AMPER;
        break;
    case '|':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_PIPE_EQUAL;
        else if (piSourceMatch(source, '|'))
            result = PI_TOKEN_PUNCT_PIPE_PIPE;
        else
            result = PI_TOKEN_PUNCT_PIPE;
        break;
    case '^':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_CARET_EQUAL;
        else
            result = PI_TOKEN_PUNCT_CARET;
        break;

    case '=':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_EQUAL_EQUAL;
        else if (piSourceMatch(source, '>'))
            result = PI_TOKEN_PUNCT_THICK_ARROW;
        else
            result = PI_TOKEN_PUNCT_EQUAL;
        break;

    case '<':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_LESS_EQUAL;
        else if (piSourceMatch(source, '<'))
            if (piSourceMatch(source, '='))
                result = PI_TOKEN_PUNCT_LESS_LESS_EQUAL;
            else
                result = PI_TOKEN_PUNCT_LESS_LESS;
        else if (piSourceMatch(source, '-'))
            result = PI_TOKEN_PUNCT_LEFT_ARROW;
        else
            result = PI_TOKEN_PUNCT_LESS;
        break;
    case '>':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_GREAT_EQUAL;
        else if (piSourceMatch(source, '>'))
            if (piSourceMatch(source, '='))
                result = PI_TOKEN_PUNCT_GREAT_GREAT_EQUAL;
            else
                result = PI_TOKEN_PUNCT_GREAT_GREAT;
        else
            result = PI_TOKEN_PUNCT_GREAT;
        break;

    case '+':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_PLUS_EQUAL;
        else if (piSourceMatch(source, '+'))
            result = PI_TOKEN_PUNCT_PLUS_PLUS;
        else
            result = PI_TOKEN_PUNCT_PLUS;
        break;
    case '-':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_MINUS_EQUAL;
        else if (piSourceMatch(source, '-'))
            result = PI_TOKEN_PUNCT_MINUS_MINUS;
        else if (piSourceMatch(source, '>'))
            result = PI_TOKEN_PUNCT_ARROW;
        else
            result = PI_TOKEN_PUNCT_MINUS;
        break;
    case '*':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_STAR_EQUAL;
        else if (piSourceMatch(source, '*'))
            result = PI_TOKEN_PUNCT_STAR_STAR;
        else
            result = PI_TOKEN_PUNCT_STAR;
        break;
    case '/':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_SLASH_EQUAL;
        else if (piSourceMatch(source, '/'))
            result = pi_ScanSingleLineComment(source, outToken);
        else if (piSourceMatch(source, '*'))
            result = pi_ScanMultiLineComment(source, outToken);
        else
            result = PI_TOKEN_PUNCT_SLASH;
        break;
    case '%':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_PERC_EQUAL;
        else
            result = PI_TOKEN_PUNCT_PERC;
        break;

    case '.':
        if (piSourceMatch(source, '.'))
            if (piSourceMatch(source, '.'))
                result = PI_TOKEN_PUNCT_DOT_DOT_DOT;
            else
                result = PI_TOKEN_PUNCT_DOT_DOT;
        else
            result = PI_TOKEN_PUNCT_DOT;
        break;
    case ':':
        if (piSourceMatch(source, '='))
            result = PI_TOKEN_PUNCT_COLON_EQUAL;
        else if (piSourceMatch(source, ':'))
            result = PI_TOKEN_PUNCT_COLON_COLON;
        else
            result = PI_TOKEN_PUNCT_COLON;
        break;
    case ';':
        if (piSourceMatch(source, ';'))
            result = PI_TOKEN_PUNCT_SEMIC_SEMIC;
        else
            result = PI_TOKEN_PUNCT_SEMIC;
        break;
    case ',':
        result = PI_TOKEN_PUNCT_COMMA;
        break;

    case '#':
        if (piSourceMatch(source, '#'))
            result = PI_TOKEN_PUNCT_HASH_HASH;
        else
            result = PI_TOKEN_PUNCT_HASH;
        break;
    case '@':
        result = PI_TOKEN_PUNCT_AT;
        break;

        /**
         * +---------------------------------------+
         */

    default:
        /* ToDo: Report diagnositc */
        result = PI_TOKEN_INVAL;
        break;
    }

    return pi_MakeToken(outToken, result, source);
}

PiTokenCode piScanToken(PiSource *const source, PiToken *const outToken)
{
    assert(source != NULL);

    PiTokenCode result;

    while (piSourceCheckPred(source, (const PiPredicateFn)isspace))
        piSourceRead(source);

    piSourceAdvance(source);

    if (piSourceCheck(source, EOF))
        result = pi_MakeEndOfFileToken(outToken, source);
    else if (piSourceCheckPred(source, (const PiPredicateFn)isdigit))
        result = pi_ScanNumericLiteral(source, outToken);
    else if (piSourceCheckPred(source, pi_IsIdentStart))
        result = pi_ScanIdentifierOrKeyword(source, outToken);
    else
        result = pi_ScanPunctuator(source, outToken);

    return result;
}

/* =------------------------------------------------------------= */
