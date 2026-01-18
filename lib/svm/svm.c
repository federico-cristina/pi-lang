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

    PiValue
        /* Unary operations register */
        val = PI_NULL,
        /* Binary left-hand value register */
        lhs = PI_NULL,
        /* Binary right-hand value register */
        rhs = PI_NULL,
        /* Operation answer register */
        ans = PI_NULL;

    /* The result of this function */
    int exitCode = EXIT_SUCCESS;

    /* Sets the Stack-VM error handler onto handlers stack */
    pi_SetErrorHandler(&svmJmpBuf);

    /* Recovery code */
    if (setjmp(svmJmpBuf) != 0)
    {
        /* Does nothing */
    }

    /* Stack-VM loop */
    do
    {
        /* Fetch opcode */
        const PiSvmOpCode opcode = *(ip++);

        /* Dispatch opcode (decode + execute) */
        switch (opcode)
        {
        case PI_SVM_OP_NOP:
            /* Does nothing */
            break;

        case PI_SVM_OP_EXIT:
            exitCode = *ip;
            goto _L_Exit;
        case PI_SVM_OP_RET:
            if (!empty())
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

        case PI_SVM_OP_LDC:
            push(K(*(ip++)));
            break;
        case PI_SVM_OP_LDI:
            push(I(*(ip++)));
            break;

#pragma endregion

            /**
             * +---- STACK OPCODES --------------------+
             */

#pragma region STACK OPCODES

        case PI_SVM_OP_POP:
            pop();
            break;
        case PI_SVM_OP_DUP:
            push(peek());
            break;

#pragma endregion

            /**
             * +---- BITWISE OPCODES ------------------+
             */
            
#pragma region BITWISE OPCODES

        case PI_SVM_OP_BNOT:
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

        case PI_SVM_OP_BAND:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
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
        case PI_SVM_OP_BOR:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
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
        case PI_SVM_OP_BXOR:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
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

        case PI_SVM_OP_NOT:
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

        case PI_SVM_OP_AND:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_NONE):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_BOOL):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_CHAR):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_UINT):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_SINT):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_REAL):
                ans = PI_FALSE;
                break;

            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_NONE):
                ans = PI_FALSE;
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_BOOL):
                ans = piBool(piAsBool(lhs) && piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_CHAR):
                ans = piBool(piAsBool(lhs) && (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_UINT):
                ans = piBool(piAsBool(lhs) && (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_SINT):
                ans = piBool(piAsBool(lhs) && (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_REAL):
                ans = piBool(piAsBool(lhs) && (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_NONE):
                ans = PI_FALSE;
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsChar(rhs) && piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsChar(rhs) && (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsChar(rhs) && (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsChar(rhs) && (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsChar(rhs) && (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_NONE):
                ans = PI_FALSE;
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsUInt(rhs) && piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsUInt(rhs) && (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsUInt(rhs) && (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsUInt(rhs) && (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsUInt(rhs) && (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_NONE):
                ans = PI_FALSE;
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsSInt(rhs) && piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsSInt(rhs) && (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsSInt(rhs) && (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsSInt(rhs) && (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsSInt(rhs) && (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_NONE):
                ans = PI_FALSE;
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsReal(rhs) && piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsReal(rhs) && (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsReal(rhs) && (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsReal(rhs) && (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsReal(rhs) && (bool)piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        case PI_SVM_OP_OR:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_NONE):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_BOOL):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_CHAR):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_UINT):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_SINT):
            case comb(PI_VALUE_TYPE_NONE, PI_VALUE_TYPE_REAL):
                ans = piBool(piIsTrue(rhs));
                break;

            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_NONE):
                ans = lhs;
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_BOOL):
                ans = piBool(piAsBool(lhs) || piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_CHAR):
                ans = piBool(piAsBool(lhs) || (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_UINT):
                ans = piBool(piAsBool(lhs) || (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_SINT):
                ans = piBool(piAsBool(lhs) || (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_BOOL, PI_VALUE_TYPE_REAL):
                ans = piBool(piAsBool(lhs) || (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_NONE):
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_NONE):
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_NONE):
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_NONE):
                ans = piBool(piIsTrue(lhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsChar(rhs) || piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsChar(rhs) || (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsChar(rhs) || (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsChar(rhs) || (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_CHAR, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsChar(rhs) || (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsUInt(rhs) || piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsUInt(rhs) || (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsUInt(rhs) || (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsUInt(rhs) || (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsUInt(rhs) || (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsSInt(rhs) || piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsSInt(rhs) || (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsSInt(rhs) || (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsSInt(rhs) || (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsSInt(rhs) || (bool)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_BOOL):
                ans = piBool((bool)piAsReal(rhs) || piAsBool(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_CHAR):
                ans = piBool((bool)piAsReal(rhs) || (bool)piAsChar(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piBool((bool)piAsReal(rhs) || (bool)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piBool((bool)piAsReal(rhs) || (bool)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piBool((bool)piAsReal(rhs) || (bool)piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

#pragma endregion

            /**
             * +---- ARITHMETIC OPCODES ---------------+
             */

#pragma region ARITHMETIC OPCODES

        case PI_SVM_OP_NEG:
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

        case PI_SVM_OP_ADD:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
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

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) + piAsUInt(rhs));
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
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) + piAsSInt(rhs));
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
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(piAsReal(lhs) + piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }

        _L_PushAns:
            push(ans);

            break;
        case PI_SVM_OP_SUB:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
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

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) - piAsUInt(rhs));
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
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) - piAsSInt(rhs));
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
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(piAsReal(lhs) - piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }

            goto _L_PushAns;
        case PI_SVM_OP_MUL:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) * piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) * (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) * (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) * (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) * piAsSInt(rhs));
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
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(piAsReal(lhs) * piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        case PI_SVM_OP_DIV:
            rhs = pop();
            lhs = pop();

            /* Checks if rhs is zero */
            if (!piIsTrue(rhs))
                ans = piReal(NAN);

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) / piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) / (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) / (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) / (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) / piAsSInt(rhs));
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
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(piAsReal(lhs) / piAsReal(rhs));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        case PI_SVM_OP_POW:
            rhs = pop();
            lhs = pop();

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
                double d_ans;

            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                d_ans = pow((double)piAsUInt(lhs), (double)piAsUInt(rhs));
                goto _L_RoundUInt;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                d_ans = pow((double)piAsUInt(lhs), (double)piAsSInt(rhs));
                goto _L_RoundUInt;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                d_ans = pow((double)piAsUInt(lhs), piAsReal(rhs));
                goto _L_RoundUInt;

            _L_RoundUInt:
                ans = piUInt((pi_uint_t)llround(d_ans));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                d_ans = pow((double)piAsSInt(lhs), (double)piAsUInt(rhs));
                goto _L_RoundSInt;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                d_ans = pow((double)piAsSInt(lhs), (double)piAsSInt(rhs));
                goto _L_RoundSInt;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_REAL):
                d_ans = pow((double)piAsSInt(lhs), piAsReal(rhs));
                goto _L_RoundSInt;
                
            _L_RoundSInt:
                ans = piSInt((pi_sint_t)llround(d_ans));
                break;

            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_UINT):
                ans = piReal(pow(piAsReal(lhs), (double)piAsUInt(rhs)));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_SINT):
                ans = piReal(pow(piAsReal(lhs), (double)piAsSInt(rhs)));
                break;
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(pow(piAsReal(lhs), piAsReal(rhs)));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;
        case PI_SVM_OP_REM:
            rhs = pop();
            lhs = pop();

            /* Checks if rhs is zero */
            if (!piIsTrue(rhs))
                ans = piReal(NAN);

            switch (comb(piValueTypeOf(lhs), piValueTypeOf(rhs)))
            {
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_UINT):
                ans = piUInt(piAsUInt(lhs) % piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_SINT):
                ans = piUInt(piAsUInt(lhs) % (pi_uint_t)piAsSInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_UINT, PI_VALUE_TYPE_REAL):
                ans = piUInt(piAsUInt(lhs) % (pi_uint_t)piAsReal(rhs));
                break;

            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_UINT):
                ans = piSInt(piAsSInt(lhs) % (pi_sint_t)piAsUInt(rhs));
                break;
            case comb(PI_VALUE_TYPE_SINT, PI_VALUE_TYPE_SINT):
                ans = piSInt(piAsSInt(lhs) % piAsSInt(rhs));
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
            case comb(PI_VALUE_TYPE_REAL, PI_VALUE_TYPE_REAL):
                ans = piReal(fmod(piAsReal(lhs), piAsReal(rhs)));
                break;

            default:
                piNotImpl();
                break;
            }
            
            goto _L_PushAns;

#pragma endregion

            /**
             * +---- I/O OPCODES ----------------------+
             */

#pragma region I/O OPCODES

        case PI_SVM_OP_OUT:
            piPrintValue(pop());
            break;

#pragma endregion

            /**
             * +---------------------------------------+
             */

        default:
            piRaiseError("illegal Stack-VM opcode " PI_ErroneousColor2("%02" PRIX8), (uint8_t)opcode);
            break;
        }
    } while (true);

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

    const int
        result = pi_SvmLoop(env, chunk, &evalStack);

    piFreeValueArray(&evalStack);

    return result;
}

/* =------------------------------------------------------------= */
