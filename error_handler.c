#include "error_handler.h" /* Includes stdio.h, stdbool.h, and definitions.h */

/*
 * @brief Internal flag to track if any errors have been reported during the current file's processing.
 * This flag determines whether output files should be generated at the end of the assembly process.
 */
static int g_errors_occurred = FALSE;

/*
 * @brief Array of detailed error messages corresponding to the ErrorType enumeration.
 * Each message provides a clear description of the error to the user.
 * The order of messages must strictly match the order of enumeration members in ErrorType.
 */
static const char* error_messages[] = {
    "No error detected.", /* ERROR_NONE */
    "Failed to open source or output file.", /* ERROR_FILE_OPEN_FAILED */
    "Source line exceeds maximum allowed length (MAX_LINE_LENGTH).", /* ERROR_LINE_TOO_LONG */
    "Input file is empty or contains only comment lines.", /* ERROR_EMPTY_OR_COMMENT_FILE */
    "Macro name is a reserved keyword (opcode, directive, or register).", /* ERROR_MACRO_NAME_RESERVED_KEYWORD */
    "Macro name does not follow legal label format (e.g., starts with a digit, too long).", /* ERROR_MACRO_NAME_INVALID_FORMAT */
    "Syntax error in 'mcro' definition line (e.g., extra characters).", /* ERROR_MACRO_DEFINITION_SYNTAX */
    "Nested macro definitions are not allowed.", /* ERROR_NESTED_MACRO_DEFINITION */
    "Unexpected 'mcroend' encountered without a preceding 'mcro' definition.", /* ERROR_UNEXPECTED_MACRO_END */
    "End of file reached before 'mcroend' was found for an open macro definition.", /* ERROR_UNCLOSED_MACRO_DEFINITION */
    "Label is defined more than once in the file.", /* ERROR_LABEL_REDEFINITION */
    "Label name is a reserved keyword (opcode, directive, or register).", /* ERROR_LABEL_RESERVED_KEYWORD */
    "Label name does not meet the specified format (e.g., starts with a digit, too long).", /* ERROR_LABEL_INVALID_FORMAT */
    "Symbol declared as both '.entry' and '.extern' in the same file.", /* ERROR_ENTRY_EXTERN_CONFLICT */
    "Use of an undefined label/symbol (detected in second pass).", /* ERROR_UNDEFINED_LABEL */
    "'.entry' directive refers to a symbol not defined in the current file (detected in second pass).", /* ERROR_ENTRY_UNDEFINED_SYMBOL */
    "Unknown instruction or directive name.", /* ERROR_UNKNOWN_OPERATION */
    "Incorrect number of operands for a given instruction.", /* ERROR_ILLEGAL_OPERAND_COUNT */
    "Operand uses an addressing method not valid for the instruction.", /* ERROR_INVALID_ADDRESSING_METHOD */
    "Immediate operand value is not a valid integer or out of range.", /* ERROR_INVALID_IMMEDIATE_VALUE */
    "'.string' directive requires the string to be enclosed in double quotes.", /* ERROR_MISSING_QUOTES_STRING */
    "Syntax error in '.data', '.string', or '.mat' directives (e.g., leading comma, non-numeric values).", /* ERROR_DATA_DIRECTIVE_SYNTAX */
    "Syntax error in matrix addressing (e.g., missing brackets, non-register index).", /* ERROR_MATRIX_INDEX_SYNTAX */
    "Register number is not within the valid range (r0-r7).", /* ERROR_REGISTER_OUT_OF_RANGE */
    "Total instruction or data image size exceeds available memory.", /* ERROR_MEMORY_OVERFLOW */
    "General syntax error not covered by a more specific type.", /* ERROR_GENERIC_SYNTAX_ERROR */
    "An unexpected internal error occurred in the assembler logic." /* ERROR_INTERNAL_ERROR */
};

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
void report_error(const char* file_name, int line_number, ErrorType error_type) {
    /* Validate the error_type to prevent out-of-bounds access to the error_messages array. */
    if (error_type >= ERROR_NONE && error_type <= ERROR_INTERNAL_ERROR) {
        /* Print the formatted error message to the standard error stream. */
        fprintf(stderr, "Error in file '%s', line %d: %s\n",
                file_name, line_number, error_messages[error_type]);
        /* Set the global error flag to indicate that at least one error has occurred. */
        g_errors_occurred = TRUE;
    } else {
        /* If an unrecognized error type is passed, report a generic internal error. */
        fprintf(stderr, "Error: An unknown error type (%d) occurred in file '%s', line %d.\n",
                error_type, file_name, line_number);
        g_errors_occurred = TRUE;
    }
}

/**
 * @brief Checks if any errors have been reported during the current assembly process for the file being processed.
 * This function is typically called at the end of the assembly passes to determine
 * whether output files (.ob, .ent, .ext) should be generated.
 *
 * @return TRUE if one or more errors have been reported, FALSE otherwise.
 */
int has_errors() {
    /* Return the current state of the global error flag. */
    return g_errors_occurred;
}

/**
 * @brief Resets the internal error flag.
 * This function should be called at the beginning of processing each new assembly file
 * to ensure that the error state is clear for the current file.
 */
void reset_error_flag() {
    /* Clear the global error flag. */
    g_errors_occurred = FALSE;
}