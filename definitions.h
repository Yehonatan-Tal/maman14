#ifndef ASSEMBLER_DEFINITIONS_H
#define ASSEMBLER_DEFINITIONS_H

/* --- General Constants --- */
/**
 * @brief Maximum line length in an assembly source file, excluding the newline character.
 * As specified, a line longer than 80 characters should report an error .
 */
#define MAX_LINE_LENGTH 80

/**
 * @brief Maximum allowed length for a legal label (symbol) [3].
 * A label can start with an alphabetic character and be followed by alphanumeric characters [3].
 */
#define MAX_LABEL_LENGTH 30

/**
 * @brief The total size of the imaginary computer's memory in cells (words).
 * The memory addresses range from 0 to 255 .*/
#define MEMORY_SIZE 256

/**
 * @brief The number of general-purpose registers in the imaginary CPU .
 * The registers are named r0 to r7 .*/
#define NUM_REGISTERS 8

/* --- Boolean Constants for C90 Compatibility --- */
#define TRUE 1
#define FALSE 0

/* --- File Extension Constants --- */
#define AS_EXTENSION ".as"
#define AM_EXTENSION ".am"
#define AS_EXTENSION_LENGTH 3
#define AM_EXTENSION_LENGTH 3

/* --- Constants for "Unique Base 4" Encoding --- */
/**
 * @brief Character representation for binary value 0 in unique base 4 .
 */
#define BASE4_A 'a'

/**
 * @brief Character representation for binary value 1 in unique base 4 .
 */
#define BASE4_B 'b'

/**
 * @brief Character representation for binary value 2 in unique base 4 .
 */
#define BASE4_C 'c'

/**
 * @brief Character representation for binary value 3 in unique base 4 .
 */
#define BASE4_D 'd'

/* --- ARE (Absolute, Relocatable, External) Encoding Types --- */
/**
 * @brief Defines the Address Relocation and External (ARE) type for machine code words.
 * This field is added only to instruction encodings (not data).
 * It consists of 2 bits in the machine code word.
 */
typedef enum {
    ARE_ABSOLUTE = 0,   /* 00 - Content of the word does not depend on its actual load address [8-10]. */
    ARE_EXTERNAL = 1,   /* 01 - Content of the word depends on an external symbol's value [8, 11, 12]. */
    ARE_RELOCATABLE = 2 /* 10 - Content of the word depends on its actual load address; requires relocation [8-11]. */
} ARE_Type;

/* --- Opcode Definitions --- */
/**
 * @brief Defines the opcode for each assembly instruction.
 * Opcodes are represented by bits 6-9 in the first word of an instruction's machine code.
 * There are 16 opcodes in our assembly language.
 */
typedef enum {
    OPCODE_MOV = 0,  /* Move (2 operands) */
    OPCODE_CMP = 1,  /* Compare (2 operands)*/
    OPCODE_ADD = 2,  /* Add (2 operands) */
    OPCODE_SUB = 3,  /* Subtract (2 operands) */
    OPCODE_NOT = 4,  /* Bitwise NOT (1 operand)*/
    OPCODE_CLR = 5,  /* Clear (1 operand)  */
    OPCODE_LEA = 6,  /* Load Effective Address (2 operands) */
    OPCODE_INC = 7,  /* Increment (1 operand) */
    OPCODE_DEC = 8,  /* Decrement (1 operand) */
    OPCODE_JMP = 9,  /* Jump (1 operand)*/
    OPCODE_BNE = 10, /* Branch if Not Equal (1 operand)*/
    OPCODE_RED = 11, /* Read (1 operand)*/
    OPCODE_PRN = 12, /* Print (1 operand) */
    OPCODE_JSR = 13, /* Jump to Subroutine (1 operand) */
    OPCODE_RTS = 14, /* Return from Subroutine (no operands) */
    OPCODE_STOP = 15 /* Stop program execution (no operands)  */
} Opcode;

/* --- Addressing Method Definitions --- */
/**
 * @brief Defines the addressing methods used for operands .
 * There are 4 addressing methods in our assembly language, numbered 0-3 .
 * Usage of addressing methods may require additional information words.
 */
typedef enum {
    ADDRESSING_IMMEDIATE = 0,       /* Immediate addressing: #VALUE  */
    ADDRESSING_DIRECT = 1,          /* Direct addressing: LABEL  */
    ADDRESSING_MATRIX = 2,          /* Matrix addressing: LABEL[rX][rY]  */
    ADDRESSING_REGISTER_DIRECT = 3  /* Register direct addressing: rX  */
} AddressingMethod;

/* --- Placeholder for future Data Structures --- */

/**
 * @brief Structure to represent a symbol (label) in the symbol table.
 * Will likely contain:
 * - char* name: The name of the symbol .
 * - int address: The memory address associated with the symbol (IC for code, DC for data).
 * - Symbol_Attribute type: e.g., CODE, DATA, ENTRY, EXTERNAL .
 */
/* typedef struct Symbol { */
/*     // ... members for symbol name, address, and attributes */
/* } Symbol; */

/**
 * @brief Structure to represent a machine instruction word.
 * This might be used to construct the machine code image word by word.
 * Will likely contain fields for:
 * - opcode, source operand addressing, destination operand addressing, ARE type .
 * - Potentially other fields for additional words (immediate value, register numbers, matrix indices) [23, 26, 30, 33].
 */
/* typedef struct Instruction_Word { */
/*     // ... members for bits representing opcode, addressing modes, ARE, etc. */
/* } Instruction_Word; */

#endif /* ASSEMBLER_DEFINITIONS_H */