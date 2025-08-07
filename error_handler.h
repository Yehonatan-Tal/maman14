#ifndef ASSEMBLER_ERROR_HANDLER_H
#define ASSEMBLER_ERROR_HANDLER_H

/* Include necessary standard libraries */
#include <stdio.h>   /* For fprintf, stderr operations */

/* Include global definitions for consistency across the project. */
#include "definitions.h" /* Includes constants like MAX_LINE_LENGTH, etc. */

/**
 * @brief This header file declares functions for managing error reporting within the assembler project.
 * Its purpose is to provide a standardized interface to report detailed error messages to standard output (stderr),
 * including the file name and the line number where the error was detected.
 * This module ensures that the assembler continues scanning the input to detect additional errors
 * instead of stopping at the first one, and indicates whether output files should be generated.
 */

/* --- Error Type Definitions --- */

/**
 * @brief Enumeration of specific error codes for different types of errors
 * that can occur during the assembly process. This helps in categorizing
 * errors and providing precise, detailed messages.
 */
typedef enum {
    ERROR_NONE = 0,                         /* No error detected */

    /* General file and I/O errors */
    ERROR_FILE_OPEN_FAILED,                 /* Failed to open the source or output file */
    ERROR_LINE_TOO_LONG,                    /* Source line exceeds the maximum allowed length (MAX_LINE_LENGTH) */
    ERROR_EMPTY_OR_COMMENT_FILE,            /* Input file is empty or contains only comments */

    /* Pre-assembler (Macro) related errors */
    ERROR_MACRO_NAME_RESERVED_KEYWORD,      /* Macro name is a reserved keyword (opcode, directive, or register) */
    ERROR_MACRO_NAME_INVALID_FORMAT,        /* Macro name does not follow legal label format (e.g., starts with a digit, too long) */
    ERROR_MACRO_DEFINITION_SYNTAX,          /* Syntax error in a 'mcro' definition line (e.g., extra characters) */
    ERROR_NESTED_MACRO_DEFINITION,          /* Nested macro definitions are not allowed */
    ERROR_UNEXPECTED_MACRO_END,             /* 'mcroend' encountered without a preceding 'mcro' */
    ERROR_UNCLOSED_MACRO_DEFINITION,        /* End of file reached before 'mcroend' was found for an open macro definition */

    /* Label/Symbol related errors */
    ERROR_LABEL_REDEFINITION,               /* Label is defined more than once in the file */
    ERROR_LABEL_RESERVED_KEYWORD,           /* Label name is a reserved keyword (opcode, directive, or register) */
    ERROR_LABEL_INVALID_FORMAT,             /* Label name does not meet the specified format (e.g., starts with a digit, too long) */
    ERROR_ENTRY_EXTERN_CONFLICT,            /* Symbol declared as both '.entry' and '.extern' in the same file */
    ERROR_UNDEFINED_LABEL,                  /* Use of an undefined label/symbol (detected in second pass) */
    ERROR_ENTRY_UNDEFINED_SYMBOL,           /* '.entry' directive refers to a symbol not defined in the current file (detected in second pass) */

    /* Instruction and Directive related errors */
    ERROR_UNKNOWN_OPERATION,                /* Unknown instruction or directive name */
    ERROR_ILLEGAL_OPERAND_COUNT,            /* Incorrect number of operands for a given instruction */
    ERROR_INVALID_ADDRESSING_METHOD,        /* Operand uses an addressing method not valid for the instruction */
    ERROR_INVALID_IMMEDIATE_VALUE,          /* Immediate operand value is not a valid integer or out of range */
    ERROR_MISSING_QUOTES_STRING,            /* '.string' directive requires the string to be enclosed in double quotes */
    ERROR_DATA_DIRECTIVE_SYNTAX,            /* Syntax error in '.data', '.string', or '.mat' directives (e.g., leading comma, non-numeric values) */
    ERROR_MATRIX_INDEX_SYNTAX,              /* Syntax error in matrix addressing (e.g., missing brackets, non-register index) */
    ERROR_REGISTER_OUT_OF_RANGE,            /* Register number is not within the valid range (r0-r7) */
    ERROR_MEMORY_OVERFLOW,                  /* Total instruction or data image size exceeds available memory */

    /* Generic and Internal Errors */
    ERROR_GENERIC_SYNTAX_ERROR,             /* General syntax error not covered by a more specific type */
    ERROR_INTERNAL_ERROR                    /* An unexpected internal error in the assembler logic */

} ErrorType;

/* --- Error Handling Core Functions --- */

/**
 * @brief Reports an error by printing a detailed error message to standard error (stderr).
 * The message includes the file name, line number, and a description corresponding to the error type.
 * This function updates an internal flag to indicate that errors have occurred,
 * which prevents the generation of output files at the end of the assembly process.
 *
 * @param file_name The base name of the source file (e.g., "my_program") where the error occurred.
 * @param line_number The 1-based line number in the source file where the error was detected.
 * @param error_type The type of error that occurred, from the ErrorType enumeration.
 */
void report_error(const char* file_name, int line_number, ErrorType error_type);

/**
 * @brief Checks if any errors have been reported during the current assembly process for the file being processed.
 * This function is typically called at the end of the assembly passes to determine
 * whether output files (.ob, .ent, .ext) should be generated.
 *
 * @return TRUE if one or more errors have been reported, FALSE otherwise.
 */
int has_errors();

/**
 * @brief Resets the internal error flag.
 * This function should be called at the beginning of processing each new assembly file
 * to ensure that the error state is clear for the current file.
 */
void reset_error_flag();

#endif /* ASSEMBLER_ERROR_HANDLER_H */