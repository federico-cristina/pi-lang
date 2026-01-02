#ifndef _PI_SVM_OPCODE_H
#   error This file should not be included directly. Include opcode.h instead.
#endif

/* =---- Bytecode Instructions OpCodes -------------------------= */

#ifndef PI_DefineOp
#   define PI_DefineOp(name, code, mnemonic, color, ...)
#endif

/**
 * @brief   No Operation - Does nothing and proceeds to the next instruction.
 */
PI_DefineOp(OP_NOP,         0x00, "nop",        PI_WHITE)

/**
 * +---- DATA TRANSFER OPCODES ------------+
 */

/**
 * @brief   Load Constant - Loads a constant from data pool to the top of the stack.
 */
PI_DefineOp(OP_LDC,         0x10, "ldc",        PI_BOLD_BLUE)
/**
 * @brief   Load Immediate - Loads an immediate value onto the top of the stack.
 */
PI_DefineOp(OP_LDI,         0x11, "ldi",        PI_BOLD_BLUE)

/**
 * +---- BITWISE OPCODES ------------------+
 */

/**
 * @brief   Bitwise Not - Bitwise not operation: requires at least one value into the stack.
 */
PI_DefineOp(OP_BNOT,        0x30, "bnot",       PI_DARK_YELLOW)
/**
 * @brief   Bitwise And - Bitwise and operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_BAND,        0x31, "band",       PI_DARK_YELLOW)
/**
 * @brief   Bitwise Or - Bitwise or operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_BOR,         0x32, "bor",        PI_DARK_YELLOW)
/**
 * @brief   Bitwise XOr - Bitwise exclusive or operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_BXOR,        0x33, "bxor",       PI_DARK_YELLOW)

/**
 * +---- LOGIC OPCODES --------------------+
 */

/**
 * @brief   Not - Boolean not operation: requires at least one value into the stack.
 */
PI_DefineOp(OP_NOT,         0x36, "not",        PI_DARK_YELLOW)
/**
 * @brief   And - Boolean and operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_AND,         0x37, "and",        PI_DARK_YELLOW)
/**
 * @brief   Or - Boolean or operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_OR,          0x38, "or",         PI_DARK_YELLOW)

/**
 * +---- ARITHMETIC OPCODES ---------------+
 */

/**
 * @brief   Negate - Arithmetic negation operation: requires at least one value into the stack.
 */
PI_DefineOp(OP_NEG,         0x39, "neg",        PI_DARK_YELLOW)
/**
 * @brief   Add - Arithmetic addition operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_ADD,         0x3A, "add",        PI_DARK_YELLOW)
/**
 * @brief   Subtract - Arithmetic subtraction operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_SUB,         0x3B, "sub",        PI_DARK_YELLOW)
/**
 * @brief   Multiply - Arithmetic multiplication operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_MUL,         0x3C, "mul",        PI_DARK_YELLOW)
/**
 * @brief   Divide - Arithmetic division operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_DIV,         0x3D, "div",        PI_DARK_YELLOW)
/**
 * @brief   Power - Arithmetic exponentiation operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_POW,         0x3E, "pow",        PI_DARK_YELLOW)
/**
 * @brief   Remainder - Arithmetic modulus division operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_REM,         0x3F, "rem",        PI_DARK_YELLOW)

/**
 * +---------------------------------------+
 */

#undef PI_DefineOp

/* =------------------------------------------------------------= */
