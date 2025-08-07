#ifndef ASSEMBLER_UTILS_H
#define ASSEMBLER_UTILS_H

/* Include necessary standard libraries and project definitions */
#include <string.h>  /* For string manipulation functions (e.g., strlen, strcmp) */
#include <stdio.h>   /* For NULL and potentially printf in debug/error context */
#include <ctype.h>   /* For character classification functions (e.g., isalpha, isalnum) */

#include "definitions.h" /* Includes global constants like MAX_LABEL_LENGTH, BASE4_A, Opcode enum, etc. */

/**  --- File Purpose ---
 * @brief This header file declares basic and general utility functions
 * that are commonly used across different modules of the assembler.
 * Its purpose is to encapsulate reusable helper functionalities such as
 * string processing, base conversions, and various validation checks.
 * This modular design improves code organization, readability, and maintainability.
 */

/* --- General Character and String Utility Functions --- */

/**
 * @brief Checks if a given character is an alphabetic character (a-z or A-Z).
 * @param c The character to check.
 * @return TRUE if the character is alphabetic, FALSE otherwise.
 */
int is_alpha(char c);

/**
 * @brief Checks if a given character is an alphanumeric character (a-z, A-Z, 0-9, or underscore).
 * @param c The character to check.
 * @return TRUE if the character is alphanumeric or underscore, FALSE otherwise.
 */
int is_alphanumeric(char c);

/**
 * @brief Checks if a given character is a whitespace character.
 * This includes space (' '), tab ('\t'), newline ('\n'), carriage return ('\r'),
 * form feed ('\f'), and vertical tab ('\v').
 * @param c The character to check.
 * @return TRUE if the character is a whitespace, FALSE otherwise.
 */
int is_whitespace(char c);

/**
 * @brief Trims leading and trailing whitespace characters from a string.
 * This function modifies the input string in-place and returns a pointer
 * to the first non-whitespace character of the trimmed string.
 * @param str A pointer to the character array (string) to be trimmed.
 * @return A pointer to the beginning of the trimmed string.
 */
char* trim_whitespace(char* str);

/**
 * @brief Skips over any leading whitespace characters in a given string.
 * This function returns a pointer to the first non-whitespace character encountered.
 * @param str A pointer to the string to process.
 * @return A pointer to the first non-whitespace character.
 */
char* skip_whitespace(char* str);

/**
 * @brief Determines if a line from the assembly source file is empty or a comment.
 * An empty line contains only whitespace characters (or is completely blank).
 * A comment line starts with a semicolon ';'.
 * @param line The string representing the line to analyze.
 * @return TRUE if the line is empty or a comment, FALSE otherwise.
 */
int is_empty_or_comment_line(const char* line);

/**
 * @brief Determines if a line from the assembly source file is a comment line.
 * A comment line starts with a semicolon ';'.
 * @param line The string representing the line to analyze.
 * @return TRUE if the line is a comment, FALSE otherwise.
 */
int is_comment_line(const char* line);

/** --- Label and Keyword Validation Functions --- **/

/**
 * @brief Checks if a given string is a legal label (symbol) according to the assembly language rules.
 * A legal label must:
 * - Start with an alphabetic character.
 * - Be followed by zero or more alphanumeric characters or underscores.
 * - Have a maximum length defined by MAX_LABEL_LENGTH.
 * - Not be a reserved keyword (e.g., an opcode, a directive name, or a register name).
 * @param label The string to validate as a label.
 * @return TRUE if the label is legal, FALSE otherwise.
 */
int is_legal_label(const char* label);

/**
 * @brief Checks if a given string is a reserved keyword in the assembly language.
 * This includes all defined opcodes, assembly directives (.data, .string, .mat, .entry, .extern),
 * and register names (r0-r7).
 * @param str The string to check.
 * @return TRUE if the string is a reserved keyword, FALSE otherwise.
 */
int is_reserved_keyword(const char* str);

/* --- Base Conversion Functions --- */

/**
 * @brief Converts a decimal integer into its unique base 4 string representation.
 * The unique base 4 uses specific characters: 'a' for binary 00, 'b' for 01, 'c' for 10, 'd' for 11.
 * The imaginary machine's word size is 10 bits. Therefore, the output string will
 * always consist of 5 base 4 characters, padded with 'a's (representing 00) if necessary.
 * Negative numbers are handled using 2's complement representation.
 * @param decimal_value The integer value to be converted.
 * @param base4_buffer A character array (buffer) where the resulting base 4 string will be stored.
 *                     It must be large enough to hold 5 characters plus a null terminator (at least 6 bytes).
 * @return A pointer to the `base4_buffer` containing the converted string.
 */
char* convert_decimal_to_unique_base4(int decimal_value, char* base4_buffer);

/* --- Opcode and Register Lookup Functions --- */

/**
 * @brief Attempts to retrieve the numerical opcode value for a given operation name string.
 * @param op_name The string representing the assembly operation (e.g., "mov", "add").
 * @param opcode_val A pointer to an integer where the corresponding opcode value will be stored
 *                   if a match is found. This value corresponds to the Opcode enum.
 * @return TRUE if the `op_name` matches a defined opcode, FALSE otherwise.
 */
int get_opcode_value(const char* op_name, int* opcode_val);

/**
 * @brief Determines if a given string represents a valid general-purpose register name (r0 to r7).
 * If valid, it also returns the corresponding register number.
 * @param str The string to check (e.g., "r0", "r5").
 * @return The integer number of the register (0-7) if `str` is a valid register name,
 *         or -1 if it is not a valid register name.
 */
int get_register_number(const char* str);

#endif /* ASSEMBLER_UTILS_H */ 