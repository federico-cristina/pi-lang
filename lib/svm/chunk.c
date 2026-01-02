#include "pi/svm/chunk.h"

/* =---- Chunks of Bytecode ------------------------------------= */

/**
 * +---- Chunk ----------------------------+
 */

PiSvmChunk *piInitSvmChunk(PiSvmChunk *const chunk)
{
    assert(chunk != NULL);

    chunk->code = NULL;
    chunk->count = 0;
    chunk->cap = 0;

    return chunk;
}

PiSvmChunk *piFreeSvmChunk(PiSvmChunk *const chunk)
{
    assert(chunk != NULL);

    /* Releases resources used to store the actual bytecode */
    if (chunk->code)
        free(chunk->code);

    chunk->code = NULL;
    chunk->count = 0;
    chunk->cap = 0;

    return chunk;
}

#ifndef pi_GrowCap
#   define pi_GrowCap(oldCap) \
(((oldCap) < 8) ? 8 : ((oldCap) * 2))
#endif

static inline void pi_SvmChunkGrow(PiSvmChunk *const chunk)
{
    const uint32_t
        oldCap = chunk->cap,
        newCap = pi_GrowCap(oldCap);

    chunk->code = piResize(uint8_t, chunk->code, oldCap, newCap);
    chunk->cap = newCap;

    return;
}

#ifndef pi_ShouldGrow
#   define pi_ShouldGrow(cap, count, n) \
    ((cap) < ((count) + (n)))
#endif

void piSvmChunkWriteOp(PiSvmChunk *const chunk, const PiSvmOpCode opcode)
{
    assert(chunk != NULL);

    if (pi_ShouldGrow(chunk->cap, chunk->count, 1))
        pi_SvmChunkGrow(chunk);
    
    chunk->code[chunk->count++] = (uint8_t)opcode;

    return;
}

void piSvmChunkWriteShortOp(PiSvmChunk *const chunk, const PiSvmOpCode opcode, const uint8_t arg)
{
    assert(chunk != NULL);

    if (pi_ShouldGrow(chunk->cap, chunk->count, 2))
        pi_SvmChunkGrow(chunk);
    
    chunk->code[chunk->count++] = (uint8_t)opcode;
    chunk->code[chunk->count++] = arg;

    return;
}

/* =------------------------------------------------------------= */
