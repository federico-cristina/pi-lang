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
 * @brief   Exit - Terminates the execution with an exit code.
 */
PI_DefineOp(OP_EXIT,        0x0E, "exit",        PI_WHITE)
/**
 * @brief   Return - Returns the top-value of the stack from the current procedure to the
 *                   precedent. When there are no values onto the stack, it just terminates
 *                   the execution.
 * 
 * @note    When used in top-level function terminates the execution.
 */
PI_DefineOp(OP_RET,         0x0F, "ret",         PI_MAGENTA)

/**
 * +---- DATA TRANSFER OPCODES ------------+
 */

/**
 * @brief   Load Constant - Loads a constant from data pool onto the top of the stack.
 */
PI_DefineOp(OP_LDC,         0x10, "ldc",        PI_BLUE)
/**
 * @brief   Load Immediate - Loads an immediate value onto the top of the stack.
 */
PI_DefineOp(OP_LDI,         0x11, "ldi",        PI_BLUE)

/**
 * +---- STACK OPCODES --------------------+
 */

/**
 * @brief   Pop - Pops a value from the stack, discarding it.
 */
PI_DefineOp(OP_POP,         0x1A, "pop",        PI_GRAY)
/**
 * @brief   Duplicate - Pushes onto the stack a copy of the top value.
 */
PI_DefineOp(OP_DUP,         0x1B, "dup",        PI_BLUE)

/**
 * +---- JUMP OPCODES ---------------------+
 */

/**
 * @brief   Jump - Unconditional jump.
 */
PI_DefineOp(OP_JMP,         0x2A, "jmp",        PI_DARK_MAGENTA)
/**
 * @brief   Jump if Zero - Jump if the top of the stack is zero.
 */
PI_DefineOp(OP_JZ,          0x2B, "jz",         PI_DARK_MAGENTA)
/**
 * @brief   Jump if Not Zero - Jump if the top of the stack is not zero.
 */
PI_DefineOp(OP_JNZ,         0x2C, "jnz",        PI_DARK_MAGENTA)

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
 * +---- RELATIONAL OPCODES ---------------+
 */

/**
 * @brief   Equals - Object equality operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_EQ,          0x40, "eq",         PI_DARK_YELLOW)
/**
 * @brief   Not Equals - Object inequality operation: requires at least two values into the stack.
 */
PI_DefineOp(OP_NE,          0x41, "ne",         PI_DARK_YELLOW)
/**
 * @brief   Less Than - Object less than comparison: requires at least two values into the stack.
 */
PI_DefineOp(OP_LT,          0x42, "lt",         PI_DARK_YELLOW)
/**
 * @brief   Less or Equal - Object less or equal to comparison: requires at least two values into the stack.
 */
PI_DefineOp(OP_LE,          0x43, "le",         PI_DARK_YELLOW)
/**
 * @brief   Greater Than - Object greater than comparison: requires at least two values into the stack.
 */
PI_DefineOp(OP_GT,          0x44, "gt",         PI_DARK_YELLOW)
/**
 * @brief   Greater or Equal - Object greater or equal to comparison: requires at least two values into the stack.
 */
PI_DefineOp(OP_GE,          0x45, "ge",         PI_DARK_YELLOW)

/**
 * +---- I/O OPCODES ----------------------+
 */

/**
 * @brief   Output - Prints on `stdout`: requires at least one value into the stack.
 */
PI_DefineOp(OP_OUT,         0xA0, "out",        PI_BLINK PI_GRAY)

/**
 * +---------------------------------------+
 */

#undef PI_DefineOp

/* =------------------------------------------------------------= */
