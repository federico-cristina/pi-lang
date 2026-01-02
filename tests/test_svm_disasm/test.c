#include "pi/svm/disasm.h"

int main(void)
{
    pi_InitSystem();

    PiSvmChunk chunk;

    piInitSvmChunk(&chunk);

    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, 0x00);
    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, 0x01);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_ADD);

    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDC, 0x00);
    piSvmChunkWriteShortOp(&chunk, PI_SVM_OP_LDI, 0x08);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_NEG);
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_SUB);

    piDisasmSvmChunk(stdout, &chunk, "TestChunk");
    
    piSvmChunkWriteOp(&chunk, PI_SVM_OP_INVALID);

    piDisasmSvmChunk(stdout, &chunk, "TestChunk");

    piFreeSvmChunk(&chunk);

    pi_FreeSystem();

    return EXIT_SUCCESS;
}
