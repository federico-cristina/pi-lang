#include "pi/svm/chunk.h"
#include "pi/svm/disasm.h"
#include "pi/svm/svm.h"

#include "pi/runtime/env.h"

#include <stdio.h>
#include <assert.h>

static void test_dup(PiEnv *env)
{
    printf("- Test: DUP (duplicate top value)\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Stack: 42 */
    piSvmChunkWriteLdI(&chunk, 42);
    /* Stack: 42, 42 (duplicated) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DUP);
    /* Stack: 84 (42 + 42) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    /* Return 84 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 84);

    printf("    Result: %d (42 * 2)\n", result);

    piFreeSvmChunk(&chunk);

    return;
}

static void test_pop(PiEnv *env)
{
    printf("- Test: POP (discard top value)\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Stack: 100 */
    piSvmChunkWriteLdI(&chunk, 100);
    /* Stack: 100, 50 */
    piSvmChunkWriteLdI(&chunk, 50);
    /* Stack: 100 (50 popped) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_POP);
    /* Return 100 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 100);

    printf("    Result: %d (50 was discarded)\n", result);

    piFreeSvmChunk(&chunk);

    return;
}

static void test_multiple_dup(PiEnv *env)
{
    printf("- Test: Multiple DUP operations\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Stack: 5 */
    piSvmChunkWriteLdI(&chunk, 5);
    /* Stack: 5, 5 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DUP);
    /* Stack: 5, 5, 5 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DUP);
    /* Stack: 5, 10 (5+5) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    /* Stack: 15 (5+10) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    /* Return 15 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 15);

    printf("    Result: %d (5 + 5 + 5)\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_complex_stack_manipulation(PiEnv *env)
{
    printf("- Test: Complex stack manipulation\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    /* Stack: 10 */
    piSvmChunkWriteLdI(&chunk, 10);
    /* Stack: 10, 20 */
    piSvmChunkWriteLdI(&chunk, 20);
    /* Stack: 10, 20, 20 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_DUP);
    /* Stack: 10, 20 (top 20 popped) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_POP);
    /* Stack: 30 (10+20) */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    /* Return 30 */
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);
    
    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 30);

    printf("    Result: %d\n", result);

    piFreeSvmChunk(&chunk);

    return;
}

static void test_nop(PiEnv *env)
{
    printf("- Test: NOP (no operation)\n");

    PiSvmChunk chunk;
    piInitSvmChunk(&chunk);

    piSvmChunkWriteLdI(&chunk, 77);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NOP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NOP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NOP);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);
    
    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 77);

    printf("    Result: %d (NOPs had no effect)\n", result);

    piFreeSvmChunk(&chunk);
}

static void test_exit_code(PiEnv *env)
{
    printf("- Test: EXIT with specific code\n");

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    piSvmChunkWriteLdI(&chunk, 123);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    /* Print disassembled chunk */
    piDisasmSvmChunk(stdout, &chunk, __func__, 4);

    int result = piRunSvmChunk(env, &chunk);

    assert(result == 123);

    printf("    Exit code: %d\n", result);

    piFreeSvmChunk(&chunk);

    return;
}

int main(void)
{
    printf("=====================================\n");
    printf("Stack-VM Stack Operations Test Suite\n");
    printf("=====================================\n\n");

    PiEnv env;

    piInitEnv(&env);

    test_dup(&env);
    test_pop(&env);
    test_multiple_dup(&env);
    test_complex_stack_manipulation(&env);
    test_nop(&env);
    test_exit_code(&env);

    piFreeEnv(&env);

    printf("\nAll stack operation tests passed!\n");

    return 0;
}
