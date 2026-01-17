#include "pi/svm/disasm.h"

#include <ctype.h>
#include <math.h>

#ifndef PI_GetDigitsCount
#   define PI_GetDigitsCount(n) \
    ((int)ceil(log10((double)(n))))
#endif

/* =---- Stack-VM Disassembler ---------------------------------= */

void piDisasmSvmChunk(FILE *const stream, const PiSvmChunk *const chunk, const char *const name)
{
    assert(chunk != NULL);
    
    const uint32_t lineMaxDigits = chunk->lines ? PI_GetDigitsCount(chunk->lines->line) : 0;

    /* Prints the initial notice */
    fprintf(stream, "Disassembling %s:\n",
        name
    );

    uint32_t offset, tmp = 0;

    for (offset = 0; offset < chunk->count; offset++)
    {
        const PiSvmOpCode opcode = (PiSvmOpCode)chunk->code[offset];

        /* Validate SVM opcode */
        if ((opcode >= 0) && (opcode < PI_SVM_OP_COUNT))
        {
            /* Handle source code information */
            if (chunk->lines)
            {
                const PiSvmLineInfo *lineInfo = (const PiSvmLineInfo *)chunk->lines;

                while (lineInfo->prev && (offset < lineInfo->prev->offset))
                    lineInfo = lineInfo->prev;

                /* Prints line or separator */
                if (tmp != lineInfo->line)
                {
                    if (lineInfo->text)
                    {
                        const char *text = lineInfo->text;

                        while (isspace(*text))
                            ++text;

                        fprintf(stream, "\n" PI_GRAY "-- %s" PI_RESET "\n", text);
                    }

                    fprintf(stream, "  %*d", lineMaxDigits, tmp = lineInfo->line);
                }
                else
                {
                    fprintf(stream, "  %*s", lineMaxDigits, "|");
                }
            }

            /* Prints offset and opcode mnemonic */
            fprintf(stream, " [" PI_GREEN "%06" PRIX32 PI_RESET "] %s%-8s" PI_RESET,
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
                uint32_t k;

            case PI_SVM_OP_LDC:
                fprintf(stream, " " PI_CYAN "#%02" PRIX8 PI_RESET, k = chunk->code[++offset]);
                /* Prints the constant value */
                fprintf(stream, " (");
                piPrintValueTo(stream, piValueArrayGet(&chunk->data, k));
                fprintf(stream, ")");
                break;
            case PI_SVM_OP_LDI:
                fprintf(stream, " " PI_MAGENTA "%" PRIX8 PI_RESET, chunk->code[++offset]);
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
