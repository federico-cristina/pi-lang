#include "pi/svm/disasm.h"
#include "pi/svm/svm.h"

int main(void)
{
    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    piSvmChunkPushLine(&chunk, 1, "let a = 1 + 1;");

    const uint32_t k1 = piSvmChunkAddConst(&chunk, piSInt(1));

    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, k1);
    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, k1);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);

    piSvmChunkPushLine(&chunk, 2, "let b = 5 + 1;");

    const uint32_t k2 = piSvmChunkAddConst(&chunk, piSInt(5));

    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, k2);
    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, k1);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);
    
    piSvmChunkPushLine(&chunk, 4, "return b;");
    
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_RET);

    const int result = piRunSvmChunk(NULL, &chunk);

    piDisasmSvmChunk(stdout, &chunk, "TestChunk");
    
    if (result == 6)
        return EXIT_SUCCESS;
    else
        return EXIT_FAILURE;
}
