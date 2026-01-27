#include "pi/svm/svm.h"

#include <math.h>

/* =---- Stack-based Virtual Machine ---------------------------= */

static int pi_SvmLoop(PiEnv *const env, const PiSvmChunk *const chunk, PiValueArray *const evalStack)
{
    jmp_buf svmJmpBuf;
    
#pragma push_macro("push")
#pragma push_macro("peek")
#pragma push_macro("pop")

#define push(value) \
    piValueArrayPush(evalStack, value)
#define peek() \
    piValueArrayTop(evalStack)
#define pop() \
    piValueArrayPop(evalStack)

#pragma push_macro("count")
#pragma push_macro("empty")

#define count() \
    evalStack->count
#define empty() \
    (count() == 0)
    
#pragma push_macro("K")
#pragma push_macro("I")

#define K(k) \
    piValueArrayGet((PiValueArray *)&chunk->data, (uint32_t)(k))
#define I(i) \
    piUInt((pi_uint_t)(i))

#define comb(type1, type2) \
    (((uint8_t)(type1) << 4) | (uint8_t)(type2))

    /* Pointer to the next instruction to be executed */
    const uint8_t *ip = (const uint8_t *)chunk->code;

#pragma push_macro("DISPATCH")
#pragma push_macro("CASE")
#pragma push_macro("NEXT")
    
#if PI_USE_JUMP_TABLE
    /* Computed goto dispatch table (direct threading) */
    static const void* dispatch_table[PI_SVM_OP_COUNT] =
    {
        [PI_SVM_OP_NOP]     = &&_L_OP_NOP,
        [PI_SVM_OP_EXIT]    = &&_L_OP_EXIT,
        [PI_SVM_OP_RET]     = &&_L_OP_RET,

        /**
         * +---- DATA TRANSFER OPCODES ------------+
         */

        [PI_SVM_OP_LDC]     = &&_L_OP_LDC,
        [PI_SVM_OP_LDI]     = &&_L_OP_LDI,

        /**
         * +---- STACK OPCODES --------------------+
         */

        [PI_SVM_OP_POP]     = &&_L_OP_POP,
        [PI_SVM_OP_DUP]     = &&_L_OP_DUP,

        /**
         * +---- BITWISE OPCODES ------------------+
         */

        [PI_SVM_OP_BNOT]    = &&_L_OP_BNOT,
        [PI_SVM_OP_BAND]    = &&_L_OP_BAND,
        [PI_SVM_OP_BOR]     = &&_L_OP_BOR,
        [PI_SVM_OP_BXOR]    = &&_L_OP_BXOR,

        /**
         *  +---- LOGIC OPCODES --------------------+
         */

        [PI_SVM_OP_NOT]     = &&_L_OP_NOT,
        [PI_SVM_OP_AND]     = &&_L_OP_AND,
        [PI_SVM_OP_OR]      = &&_L_OP_OR,

        /**
         * +---- ARITHMETIC OPCODES ---------------+
         */

        [PI_SVM_OP_NEG]     = &&_L_OP_NEG,
        [PI_SVM_OP_ADD]     = &&_L_OP_ADD,
        [PI_SVM_OP_SUB]     = &&_L_OP_SUB,
        [PI_SVM_OP_MUL]     = &&_L_OP_MUL,
        [PI_SVM_OP_DIV]     = &&_L_OP_DIV,
        [PI_SVM_OP_POW]     = &&_L_OP_POW,
        [PI_SVM_OP_REM]     = &&_L_OP_REM,

        /**
         * +---- RELATIONAL OPCODES ---------------+
         */
        
        [PI_SVM_OP_EQ]      = &&_L_OP_EQ,
        [PI_SVM_OP_NE]      = &&_L_OP_NE,
        [PI_SVM_OP_LT]      = &&_L_OP_LT,
        [PI_SVM_OP_LE]      = &&_L_OP_LE,
        [PI_SVM_OP_GT]      = &&_L_OP_GT,
        [PI_SVM_OP_GE]      = &&_L_OP_GE,

        /**
         * +---- I/O OPCODES ----------------------+
         */

        [PI_SVM_OP_OUT]     = &&OP_OUT,

        /**
         * +---------------------------------------+
         */
    };

#   define DISPATCH() \
        goto *dispatch_table[*(ip++)]
#   define CASE(op) \
        _L_OP_ ## op:
#   define NEXT() \
        DISPATCH()
#else
    /* Traditional switch-case dispatch (fallback) */
#   define DISPATCH() \
        /* No op */
#   define CASE(op) \
        case PI_SVM_OP_ ## op:
#   define NEXT() \
        break
#endif

    PiValue
        /* Unary operations register */
        val = PI_NULL,
        /* Binary left-hand value register */
        lhs = PI_NULL,
        /* Binary right-hand value register */
        rhs = PI_NULL,
        /* Operation answer register */
        ans = PI_NULL;

    /* Type check cache variables for binary operations */
    PiValueType lhs_type, rhs_type;
    /* The result of this function */
    int exitCode = EXIT_SUCCESS;

    /* Sets the Stack-VM error handler onto handlers stack */
    piSetErrorHandler(&svmJmpBuf);

    /* Recovery code */
    if (setjmp(svmJmpBuf) != 0)
    {
        /* Does nothing */
    }

    /* Stack-VM loop */
#if PI_USE_JUMP_TABLE
    /* Computed goto dispatch: jump to first instruction */
    DISPATCH();

    CASE(NOP)
#else
    /* Traditional switch-case dispatch */
    do
    {
        /* Fetch opcode */
        const PiSvmOpCode opcode = *(ip++);

        /* Dispatch opcode (decode + execute) */
        switch (opcode)
        {
        CASE(NOP)
#endif
            /* Does nothing */
            NEXT();

        CASE(EXIT)
            exitCode = *ip;
            goto _L_Exit;
        CASE(RET)
            if (PI_LIKELY(!empty()))
            {
                val = pop();

                switch (piValueTypeOf(val))
                {
                case PI_VALUE_TYPE_NONE:
                case PI_VALUE_TYPE_BOOL:
                case PI_VALUE_TYPE_CHAR:
                case PI_VALUE_TYPE_UINT:
                case PI_VALUE_TYPE_SINT:
                    exitCode = (int)piAsSInt(val);
                    break;
                case PI_VALUE_TYPE_REAL:
                    exitCode = (int)piAsReal(val);
                    break;
                
                default:
                    piNotImpl();
                    break;
                }
            }

            goto _L_Exit;

            /**
             * +---- DATA TRANSFER OPCODES ------------+
             */

#pragma region DATA TRANSFER OPCODES

        CASE(LDC)
            push(K(*(ip++)));
            break;
        CASE(LDI)
            push(I(*(ip++)));
            break;

#pragma endregion

            /**
             * +---- STACK OPCODES --------------------+
             */

#pragma region STACK OPCODES

        CASE(POP)
            pop();
            break;
        CASE(DUP)
            push(peek());
            break;

#pragma endregion

            /**
             * +---- BITWISE OPCODES ------------------+
             */
            
#pragma region BITWISE OPCODES

        CASE(BNOT)
            val = pop();

            switch (piValueTypeOf(val))
            {
            case PI_VALUE_TYPE_UINT:
                ans = piUInt(~piAsUInt(val));
                break;
            case PI_VALUE_TYPE_SINT:
                ans = piSInt(~piAsSInt(val));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

        CASE(BAND)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) & piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) & (pi_uint_t)piAsSInt(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) & (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) & piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        CASE(BOR)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) | piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) | (pi_uint_t)piAsSInt(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) | (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) | piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        CASE(BXOR)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) ^ piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) ^ (pi_uint_t)piAsSInt(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) ^ (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) ^ piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

#pragma endregion

            /**
             * +---- LOGIC OPCODES --------------------+
             */

#pragma region LOGIC OPCODES

        CASE(NOT)
            val = pop();

            switch (piValueTypeOf(val))
            {
            case PI_VALUE_TYPE_NONE:
                ans = PI_TRUE;
                break;
            case PI_VALUE_TYPE_BOOL:
                ans = piBool(!piAsBool(val));
                break;
            case PI_VALUE_TYPE_CHAR:
                ans = piBool(!piAsChar(val));
                break;
            case PI_VALUE_TYPE_UINT:
                ans = piBool(!piAsUInt(val));
                break;
            case PI_VALUE_TYPE_SINT:
                ans = piBool(!piAsSInt(val));
                break;
            case PI_VALUE_TYPE_REAL:
                ans = piBool(!piAsReal(val));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

        CASE(AND)
            rhs = pop();
            lhs = pop();

            /* Normalize both values to boolean and compute AND */
            ans = piBool(piIsTrue(lhs) && piIsTrue(rhs));

            goto _L_PushAns;
        CASE(OR)
            rhs = pop();
            lhs = pop();

            /* Normalize both values to boolean and compute OR */
            ans = piBool(piIsTrue(lhs) || piIsTrue(rhs));

            goto _L_PushAns;

#pragma endregion

            /**
             * +---- ARITHMETIC OPCODES ---------------+
             */

#pragma region ARITHMETIC OPCODES

        CASE(NEG)
            val = pop();

            switch (piValueTypeOf(val))
            {
            case PI_VALUE_TYPE_UINT:
                ans = piSInt(-(pi_sint_t)piAsUInt(val));
                break;
            case PI_VALUE_TYPE_SINT:
                ans = piSInt(-piAsSInt(val));
                break;
            case PI_VALUE_TYPE_REAL:
                ans = piReal(-piAsReal(val));
                break;

            default:
                piNotImpl();
                break;
            }

            goto _L_PushAns;

        CASE(ADD)
            rhs = pop();
            lhs = pop();
            
            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                ans = piUInt(piAsUInt(lhs) + piAsUInt(rhs));
                goto _L_PushAns;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(piAsReal(lhs) + piAsReal(rhs));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                ans = piSInt(piAsSInt(lhs) + piAsSInt(rhs));
                goto _L_PushAns;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_CHAR):
                ans = piChar(piAsChar(lhs) + piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_UINT):
                ans = piChar(piAsChar(lhs) + (char)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_SINT):
                ans = piChar(piAsChar(lhs) + (char)piAsSInt(rhs));
                break;

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) + (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) + (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) + (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piSInt(piAsSInt(lhs) + (pi_sint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(piAsReal(lhs) + (double)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(piAsReal(lhs) + (double)piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }

        _L_PushAns:
            push(ans);

            NEXT();
        CASE(SUB)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                ans = piUInt(piAsUInt(lhs) - piAsUInt(rhs));
                goto _L_PushAns;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(piAsReal(lhs) - piAsReal(rhs));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                ans = piSInt(piAsSInt(lhs) - piAsSInt(rhs));
                goto _L_PushAns;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_CHAR):
                ans = piChar(piAsChar(lhs) - piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_UINT):
                ans = piChar(piAsChar(lhs) - (char)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_SINT):
                ans = piChar(piAsChar(lhs) - (char)piAsSInt(rhs));
                break;

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) - (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) - (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) - (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piSInt(piAsSInt(lhs) - (pi_sint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(piAsReal(lhs) - (double)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(piAsReal(lhs) - (double)piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }

            goto _L_PushAns;
        CASE(MUL)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                ans = piUInt(piAsUInt(lhs) * piAsUInt(rhs));
                goto _L_PushAns;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(piAsReal(lhs) * piAsReal(rhs));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                ans = piSInt(piAsSInt(lhs) * piAsSInt(rhs));
                goto _L_PushAns;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) * (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) * (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) * (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piSInt(piAsSInt(lhs) * (pi_sint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(piAsReal(lhs) * (double)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(piAsReal(lhs) * (double)piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        CASE(DIV)
            rhs = pop();
            lhs = pop();

            /* Checks if rhs is zero */
            if (PI_UNLIKELY(!piIsTrue(rhs)))
            {
                ans = piReal(NAN);
                goto _L_PushAns;
            }

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                ans = piUInt(piAsUInt(lhs) / piAsUInt(rhs));
                goto _L_PushAns;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(piAsReal(lhs) / piAsReal(rhs));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                ans = piSInt(piAsSInt(lhs) / piAsSInt(rhs));
                goto _L_PushAns;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) / (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) / (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) / (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piSInt(piAsSInt(lhs) / (pi_sint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(piAsReal(lhs) / (double)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(piAsReal(lhs) / (double)piAsSInt(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

            /* Double result register */
            double d_ans;

        CASE(POW)
            rhs = pop();
            lhs = pop();

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                d_ans = pow((double)piAsUInt(lhs), (double)piAsUInt(rhs));
                goto _L_RoundUInt;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(pow(piAsReal(lhs), piAsReal(rhs)));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                d_ans = pow((double)piAsSInt(lhs), (double)piAsSInt(rhs));
                goto _L_RoundSInt;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                d_ans = pow((double)piAsUInt(lhs), (double)piAsSInt(rhs));
                goto _L_RoundUInt;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                d_ans = pow((double)piAsUInt(lhs), piAsReal(rhs));
                goto _L_RoundUInt;

            _L_RoundUInt:
                ans = piUInt((pi_uint_t)llround(d_ans));
                NEXT();

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                d_ans = pow((double)piAsSInt(lhs), (double)piAsUInt(rhs));
                goto _L_RoundSInt;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                d_ans = pow((double)piAsSInt(lhs), piAsReal(rhs));
                goto _L_RoundSInt;
                
            _L_RoundSInt:
                ans = piSInt((pi_sint_t)llround(d_ans));
                NEXT();

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(pow(piAsReal(lhs), (double)piAsUInt(rhs)));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(pow(piAsReal(lhs), (double)piAsSInt(rhs)));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        CASE(REM)
            rhs = pop();
            lhs = pop();

            /* Checks if rhs is zero */
            if (PI_UNLIKELY(!piIsTrue(rhs)))
            {
                ans = piReal(NAN);
                goto _L_PushAns;
            }

            /* Cache type checks to avoid redundant calls */
            lhs_type = piValueTypeOf(lhs);
            rhs_type = piValueTypeOf(rhs);

            /* Fast path: both unsigned integers */
            if (PI_LIKELY((lhs_type == PI_VALUE_TYPE_UINT) && (rhs_type == PI_VALUE_TYPE_UINT)))
            {
                ans = piUInt(piAsUInt(lhs) % piAsUInt(rhs));
                goto _L_PushAns;
            }
            
            /* Fast path: both reals */
            if ((lhs_type == PI_VALUE_TYPE_REAL) && (rhs_type == PI_VALUE_TYPE_REAL))
            {
                ans = piReal(fmod(piAsReal(lhs), piAsReal(rhs)));
                goto _L_PushAns;
            }

            /* Fast path: both signed integers */
            if ((lhs_type == PI_VALUE_TYPE_SINT) && (rhs_type == PI_VALUE_TYPE_SINT))
            {
                ans = piSInt(piAsSInt(lhs) % piAsSInt(rhs));
                goto _L_PushAns;
            }

            /* Slow path: mixed types (fall back to exhaustive switch) */
            switch (comb(lhs_type, rhs_type))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) % (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) % (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) % (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piSInt(piAsSInt(lhs) % (pi_sint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(fmod(piAsReal(lhs), (double)piAsUInt(rhs)));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(fmod(piAsReal(lhs), (double)piAsSInt(rhs)));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

#pragma endregion

            /**
             * +---- RELATIONAL OPCODES ---------------+
             */

#pragma region RELATIONAL OPCODES

        CASE(EQ)

            NEXT();

#pragma endregion

            /**
             * +---- I/O OPCODES ----------------------+
             */

#pragma region I/O OPCODES

        CASE(OUT)
            piPrintValue(pop());
            NEXT();

#pragma endregion

            /**
             * +---------------------------------------+
             */
            
#if !PI_USE_JUMP_TABLE
        default:
            piError("illegal Stack-VM opcode " PI_ErroneousColor("%02" PRIX8), (uint8_t)opcode);
            /* Set a failure return code */
            exitCode = EXIT_FAILURE;
            /* Exits execution */
            goto _L_Exit;
        }
    } while (true);
#endif

#undef DISPATCH
#undef CASE
#undef NEXT

#pragma pop_macro("DISPATCH")
#pragma pop_macro("CASE")
#pragma pop_macro("NEXT")

#undef push
#undef peek
#undef pop

#pragma pop_macro("push")
#pragma pop_macro("peek")
#pragma pop_macro("pop")

#undef count
#undef empty

#pragma push_macro("count")
#pragma push_macro("empty")
    
#undef K
#undef I

#pragma pop_macro("K")
#pragma pop_macro("I")

_L_Exit:
    return exitCode;
}

int piRunSvmChunk(PiEnv *const env, const PiSvmChunk *const chunk)
{
    assert(chunk != NULL);

    PiValueArray evalStack;

    piInitValueArray(&evalStack, PI_DEFAULT_ARRAY_CAP);

    /* Register VM stack as GC root */
    piGcSetVmStack(env, &evalStack);

    const int
        result = pi_SvmLoop(env, chunk, &evalStack);

    /* Unregister VM stack */
    piGcSetVmStack(env, NULL);

    piFreeValueArray(&evalStack);

    return result;
}

/* =------------------------------------------------------------= */
