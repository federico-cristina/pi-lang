/**
 * @file        test.c
 *
 * @brief       Stack-VM Arithmetic Operations Test Suite
 *
 * Tests all arithmetic operations of the Stack-based Virtual Machine:
 * - Addition (ADD)
 * - Subtraction (SUB)
 * - Multiplication (MUL)
 * - Division (DIV)
 * - Negation (NEG)
 * - Power (POW)
 * - Remainder (REM)
 */

#include "pi/svm/svm.h"
#include "pi/svm/chunk.h"
#include "pi/runtime/env.h"

#include <stdio.h>
#include <assert.h>
#include <math.h>

/* Helper function to capture VM output */
static int g_lastExitCode = 0;

static void test_addition(PiEnv *env)
{
    printf("  Test: Addition (5 + 3 = 8)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 5, LDI 3, ADD, RET */
    piSvmChunkWriteLdI(&chunk, 5);
    piSvmChunkWriteLdI(&chunk, 3);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 8);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_subtraction(PiEnv *env)
{
    printf("  Test: Subtraction (10 - 4 = 6)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 10, LDI 4, SUB, RET */
    piSvmChunkWriteLdI(&chunk, 10);
    piSvmChunkWriteLdI(&chunk, 4);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_SUB);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 6);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_multiplication(PiEnv *env)
{
    printf("  Test: Multiplication (7 * 6 = 42)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 7, LDI 6, MUL, RET */
    piSvmChunkWriteLdI(&chunk, 7);
    piSvmChunkWriteLdI(&chunk, 6);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_MUL);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 42);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_division(PiEnv *env)
{
    printf("  Test: Division (20 / 4 = 5)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 20, LDI 4, DIV, RET */
    piSvmChunkWriteLdI(&chunk, 20);
    piSvmChunkWriteLdI(&chunk, 4);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DIV);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 5);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_negation(PiEnv *env)
{
    printf("  Test: Negation (-42 = -42)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 42, NEG, RET */
    piSvmChunkWriteLdI(&chunk, 42);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NEG);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == -42);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_remainder(PiEnv *env)
{
    printf("  Test: Remainder (17 %% 5 = 2)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 17, LDI 5, REM, RET */
    piSvmChunkWriteLdI(&chunk, 17);
    piSvmChunkWriteLdI(&chunk, 5);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_REM);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 2);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_power(PiEnv *env)
{
    printf("  Test: Power (2 ^ 8 = 256)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 2, LDI 8, POW, RET */
    piSvmChunkWriteLdI(&chunk, 2);
    piSvmChunkWriteLdI(&chunk, 8);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_POW);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 256);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_complex_expression(PiEnv *env)
{
    printf("  Test: Complex Expression ((5 + 3) * 2 - 4 = 12)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDI 5, LDI 3, ADD, LDI 2, MUL, LDI 4, SUB, RET */
    /* Stack: 5 */
    piSvmChunkWriteLdI(&chunk, 5);
    /* Stack: 5, 3 */
    piSvmChunkWriteLdI(&chunk, 3);
    /* Stack: 8 (5+3) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    /* Stack: 8, 2 */
    piSvmChunkWriteLdI(&chunk, 2);
    /* Stack: 16 (8*2) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_MUL);
    /* Stack: 16, 4 */
    piSvmChunkWriteLdI(&chunk, 4);
    /* Stack: 12 (16-4) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_SUB);
    /* Return 12 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 12);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_floating_point(PiEnv *env)
{
    printf("  Test: Floating Point (3.5 + 2.5 = 6.0)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    /* Bytecode: LDC 3.5, LDC 2.5, ADD, RET */
    piSvmChunkWriteLdC(&chunk, piReal(3.5));
    piSvmChunkWriteLdC(&chunk, piReal(2.5));
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    int result = piRunSvmChunk(env, &chunk);
    assert(result == 6);
    printf("    Result: %d ✓\n", result);

    piFreeSvmChunk(&chunk);
}

int main(void)
{
    printf("Stack-VM Arithmetic Operations Test Suite\n");
    printf("==========================================\n\n");

    PiEnv env;
    piInitEnv(&env);

    test_addition(&env);
    test_subtraction(&env);
    test_multiplication(&env);
    test_division(&env);
    test_negation(&env);
    test_remainder(&env);
    test_power(&env);
    test_complex_expression(&env);
    test_floating_point(&env);

    piFreeEnv(&env);

    printf("\n✓ All arithmetic tests passed!\n");
    return 0;
}
