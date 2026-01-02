#include "pi/svm/disasm.h"

/* =---- Stack-VM Disassembler ---------------------------------= */

void piDisasmSvmChunk(FILE *const stream, const PiSvmChunk *const chunk, const char *const name)
{
    assert(chunk != NULL);

    /* Prints the initial notice */
    fprintf(stream, "Disassembling %s:\n",
        name
    );

    uint32_t offset;

    for (offset = 0; offset < chunk->count; offset++)
    {
        const PiSvmOpCode opcode = (PiSvmOpCode)chunk->code[offset];

        /* Validate SVM opcode */
        if ((opcode >= 0) && (opcode < PI_SVM_OP_COUNT))
        {
            /* Prints offset and opcode mnemonic */
            fprintf(stream, "  [" PI_BOLD_DARK_GREEN "%06" PRIX32 PI_RESET "] %s%-8s" PI_RESET,
                offset,
#if PI_USE_COLORS
                piGetOpCodeColor(opcode),
#else
                "",
#endif
                piGetOpCodeName(opcode)
            );

            /* Handle opcodes with arguments */
            switch (opcode)
            {
            case PI_SVM_OP_LDC:
                fprintf(stream, " " PI_BOLD_DARK_CYAN "#%02" PRIX8 PI_RESET, chunk->code[++offset]);
                break;
            case PI_SVM_OP_LDI:
                fprintf(stream, " " PI_BOLD_MAGENTA "%" PRIX8 PI_RESET, chunk->code[++offset]);
                break;
            
            default:
                /* Does nothing */
                break;
            }

            /* Endline */
            fputc('\n', stream);
        }
        else
        {
            /* Error: Invalid opcode found! */
            piRaiseError("found and invalid opcode " PI_InvalidHexColor("0x%02" PRIX8) " during disassembling",
                (uint8_t)opcode
            );
        }
    }
    
    return;
}

/* =------------------------------------------------------------= */
