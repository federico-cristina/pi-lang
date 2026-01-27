/**
 * @file        test.c
 *
 * @brief       Stack-VM Disassembler Test Suite
 *
 * Tests the bytecode disassembler functionality:
 * - Disassembly of simple programs
 * - Disassembly of complex programs
 * - Verification of instruction mnemonics
 * - Verification of operand display
 */

#include "pi/svm/svm.h"
#include "pi/svm/chunk.h"
#include "pi/svm/disasm.h"
#include "pi/runtime/env.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>

static void test_disasm_simple_arithmetic(void)
{
    printf("  Test: Disassembly of simple arithmetic (5 + 3)\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    piSvmChunkPushLine(&chunk, 1, "5 + 3");

    piSvmChunkWriteLdI(&chunk, 5);
    piSvmChunkWriteLdI(&chunk, 3);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_with_constants(void)
{
    printf("  Test: Disassembly with constant pool\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    piSvmChunkPushLine(&chunk, 1, "pi * e");

    piSvmChunkWriteLdC(&chunk, piReal(3.14159));
    piSvmChunkWriteLdC(&chunk, piReal(2.71828));
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_MUL);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);
    
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_complex_program(void)
{
    printf("  Test: Disassembly of complex program\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Program: ((5 + 3) * 2 - 4) / 2 ^ 2 */
    piSvmChunkPushLine(&chunk, 1, "((5 + 3) * 2 - 4) / 2 ^ 2");

    /* (5 + 3) = 8 */
    piSvmChunkWriteLdI(&chunk, 5);
    piSvmChunkWriteLdI(&chunk, 3);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);

    /* 8 * 2 = 16 */
    piSvmChunkWriteLdI(&chunk, 2);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_MUL);

    /* 16 - 4 = 12 */
    piSvmChunkWriteLdI(&chunk, 4);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_SUB);

    /* 2 ^ 2 = 4 */
    piSvmChunkWriteLdI(&chunk, 2);
    piSvmChunkWriteLdI(&chunk, 2);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_POW);

    /* 12 / 4 = 3 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DIV);

    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_logical_ops(void)
{
    printf("  Test: Disassembly of logical operations\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Program: NOT (true AND false) OR true */
    piSvmChunkPushLine(&chunk, 1, "NOT (true AND false) OR true");

    /* true AND false = false */
    piSvmChunkWriteLdC(&chunk, PI_TRUE);
    piSvmChunkWriteLdC(&chunk, PI_FALSE);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_AND);

    /* NOT false = true */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NOT);

    /* true OR true = true */
    piSvmChunkWriteLdC(&chunk, PI_TRUE);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_OR);

    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_bitwise_ops(void)
{
    printf("  Test: Disassembly of bitwise operations\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Program: (0xFF & 0x0F) | (BNOT 0x00) */
    piSvmChunkPushLine(&chunk, 1, "(0xFF & 0x0F) | (~0x00)");

    /* 0xFF & 0x0F = 0x0F */
    piSvmChunkWriteLdI(&chunk, 0xFF);
    piSvmChunkWriteLdI(&chunk, 0x0F);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_BAND);

    /* ~0x00 = 0xFF */
    piSvmChunkWriteLdI(&chunk, 0x00);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_BNOT);

    /* 0x0F | 0xFF = 0xFF */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_BOR);

    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_stack_ops(void)
{
    printf("  Test: Disassembly of stack operations\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Program: DUP and POP demonstration */
    piSvmChunkPushLine(&chunk, 1, "stack operations demo");

    piSvmChunkWriteLdI(&chunk, 42);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DUP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_POP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NOP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);
    
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

static void test_disasm_multiline(void)
{
    printf("  Test: Disassembly with multiple source lines\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Line 1: x = 5 */
    piSvmChunkPushLine(&chunk, 1, "x = 5");
    piSvmChunkWriteLdI(&chunk, 5);

    /* Line 2: y = 3 */
    piSvmChunkPushLine(&chunk, 2, "y = 3");
    piSvmChunkWriteLdI(&chunk, 3);

    /* Line 3: result = x + y */
    piSvmChunkPushLine(&chunk, 3, "result = x + y");
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);

    /* Line 4: return result */
    piSvmChunkPushLine(&chunk, 4, "return result");
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    piFreeSvmChunk(&chunk);
}

int main(void)
{
    printf("=================================\n");
    printf("Stack-VM Disassembler Test Suite\n");
    printf("=================================\n\n");

    test_disasm_simple_arithmetic();
    test_disasm_with_constants();
    test_disasm_complex_program();
    test_disasm_logical_ops();
    test_disasm_bitwise_ops();
    test_disasm_stack_ops();
    test_disasm_multiline();

    printf("All disassembler tests completed!\n");
    printf("\nNote: Visual inspection of disassembly output is required.\n");
    printf("Verify that:\n");
    printf("  - Instruction mnemonics are correct\n");
    printf("  - Operands are displayed properly\n");
    printf("  - Offsets are sequential\n");
    printf("  - Source line info is shown (when available)\n");

    return 0;
}
