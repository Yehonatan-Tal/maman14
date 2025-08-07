#include "utils.h"
#include "definitions.h"

#include <string.h>  /* For strlen, strcmp */
#include <stdio.h>   /* For NULL */
#include <ctype.h>   /* For isalpha, isalnum, isspace */

/* --- Internal Helper Data for Reserved Keywords and Opcodes --- */

/* Array of all reserved opcode names (must match the order of the Opcode enum) */
static const char* opcode_names[] = {
    "mov", "cmp", "add", "sub", "not", "clr", "lea", "inc",
    "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop"
};

/* Array of all reserved directive names */
static const char* directive_names[] = {
    ".data", ".string", ".mat", ".entry", ".extern"
};

/* Array of all reserved register names */
static const char* register_names[] = {
    "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"
};

/* --- General Character and String Utility Functions --- */

/**
 * @brief Checks if a given character is an alphabetic character (a-z or A-Z).
 * @param c The character to check.
 * @return TRUE if the character is alphabetic, FALSE otherwise.
 * This function utilizes the standard `isalpha` function for character classification [1].
 */
int is_alpha(char c) {
    return isalpha((unsigned char)c) ? TRUE : FALSE;
}

/**
 * @brief Checks if a given character is an alphanumeric character (a-z, A-Z, 0-9, or underscore).
 * @param c The character to check.
 * @return TRUE if the character is alphanumeric or underscore, FALSE otherwise.
 * This function extends the standard `isalnum` function to include underscore.
 */
int is_alphanumeric(char c) {
    return (isalnum((unsigned char)c) || c == '_') ? TRUE : FALSE;
}

/**
 * @brief Checks if a given character is a whitespace character.
 * This includes space (' '), tab ('\t'), newline ('\n'), carriage return ('\r'),
 * form feed ('\f'), and vertical tab ('\v') [2].
 * @param c The character to check.
 * @return TRUE if the character is a whitespace, FALSE otherwise.
 * This function utilizes the standard `isspace` function for character classification [2].
 */
int is_whitespace(char c) {
    return isspace((unsigned char)c) ? TRUE : FALSE;
}

/**
 * @brief Trims leading and trailing whitespace characters from a string.
 * This function modifies the input string in-place and returns a pointer
 * to the first non-whitespace character of the trimmed string [2].
 * @param str A pointer to the character array (string) to be trimmed.
 * @return A pointer to the beginning of the trimmed string.
 */
char* trim_whitespace(char* str) {
    char *end;

    /* Handle NULL or empty string */
    if (str == NULL || *str == '\0') {
        return str;
    }

    /* Trim leading whitespace */
    while (is_whitespace((unsigned char)*str)) {
        str++;
    }

    /* Handle string that was all whitespace */
    if (*str == '\0') {
        return str;
    }

    /* Trim trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && is_whitespace((unsigned char)*end)) {
        end--;
    }

    /* Null-terminate the new end of the string */
    *(end + 1) = '\0';

    return str;
}

/**
 * @brief Skips over any leading whitespace characters in a given string.
 * This function returns a pointer to the first non-whitespace character encountered [3].
 * @param str A pointer to the string to process.
 * @return A pointer to the first non-whitespace character.
 */
char* skip_whitespace(char* str) {
    if (str == NULL) {
        return NULL;
    }
    while (is_whitespace((unsigned char)*str)) {
        str++;
    }
    return str;
}

/**
 * @brief Determines if a line from the assembly source file is empty or a comment.
 * An empty line contains only whitespace characters (or is completely blank) [4, 5].
 * A comment line starts with a semicolon ';' [4, 6].
 * @param line The string representing the line to analyze.
 * @return TRUE if the line is empty or a comment, FALSE otherwise.
 */
int is_empty_or_comment_line(const char* line) {
    const char* trimmed_line;
    
    if (line == NULL) {
        return TRUE; /* Treat NULL line as empty */
    }
    trimmed_line = skip_whitespace((char*)line);
    if (*trimmed_line == '\0') { /* Only whitespace or empty */
        return TRUE;
    }
    if (*trimmed_line == ';') { /* Comment line */
        return TRUE;
    }
    return FALSE;
}

/**
 * @brief Determines if a line from the assembly source file is a comment line.
 * A comment line starts with a semicolon ';' [4, 6].
 * @param line The string representing the line to analyze.
 * @return TRUE if the line is a comment, FALSE otherwise.
 */
int is_comment_line(const char* line) {
    const char* trimmed_line;
    
    if (line == NULL) {
        return FALSE; /* Treat NULL line as not a comment */
    }
    trimmed_line = skip_whitespace((char*)line);
    if (*trimmed_line == ';') { /* Comment line */
        return TRUE;
    }
    return FALSE;
}

/* --- Label and Keyword Validation Functions --- */

/**
 * @brief Checks if a given string is a reserved keyword in the assembly language.
 * This includes all defined opcodes, assembly directives (.data, .string, .mat, .entry, .extern),
 * and register names (r0-r7) [7, 8].
 * @param str The string to check.
 * @return TRUE if the string is a reserved keyword, FALSE otherwise.
 */
int is_reserved_keyword(const char* str) {
    unsigned int i;

    /* Check opcodes [9-11] */
    for (i = 0; i < sizeof(opcode_names) / sizeof(opcode_names[0]); i++) {
        if (strcmp(str, opcode_names[i]) == 0) {
            return TRUE;
        }
    }

    /* Check directives */
    for (i = 0; i < sizeof(directive_names) / sizeof(directive_names[0]); i++) {
        if (strcmp(str, directive_names[i]) == 0) {
            return TRUE;
        }
    }

    /* Check registers [12, 13] */
    for (i = 0; i < sizeof(register_names) / sizeof(register_names[0]); i++) {
        if (strcmp(str, register_names[i]) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

/**
 * @brief Checks if a given string is a legal label (symbol) according to the assembly language rules.
 * A legal label must:
 * - Start with an alphabetic character [7, 12, 14].
 * - Be followed by zero or more alphanumeric characters or underscores [7, 12, 14].
 * - Have a maximum length defined by MAX_LABEL_LENGTH (30 characters) [7, 12, 14].
 * - Not be a reserved keyword (e.g., an opcode, a directive name, or a register name) [7, 14, 15].
 * @param label The string to validate as a label.
 * @return TRUE if the label is legal, FALSE otherwise.
 */
int is_legal_label(const char* label) {
    int i;
    
    if (label == NULL || *label == '\0') {
        return FALSE; /* Label cannot be empty */
    }

    /* Rule 1: Must start with an alphabetic character */
    if (!is_alpha(*label)) {
        return FALSE;
    }

    /* Rule 2: Followed by zero or more alphanumeric characters */
    for (i = 1; label[i] != '\0'; i++) {
        if (!is_alphanumeric(label[i])) {
            return FALSE;
        }
    }

    /* Rule 3: Maximum length defined by MAX_LABEL_LENGTH */
    if (strlen(label) > MAX_LABEL_LENGTH) {
        return FALSE;
    }

    /* Rule 4: Not a reserved keyword */
    if (is_reserved_keyword(label)) {
        return FALSE;
    }

    return TRUE;
}

/* --- Base Conversion Functions --- */

/**
 * @brief Converts a decimal integer into its unique base 4 string representation.
 * The unique base 4 uses specific characters: 'a' for binary 00, 'b' for 01, 'c' for 10, 'd' for 11 [8, 16-19].
 * The imaginary machine's word size is 10 bits [18, 20-22]. Therefore, the output string will
 * always consist of 5 base 4 characters, padded with 'a's (representing 00) if necessary [18-20].
 * Negative numbers are handled using 2's complement representation [20, 21].
 * @param decimal_value The integer value to be converted.
 * @param base4_buffer A character array (buffer) where the resulting base 4 string will be stored.
 * It must be large enough to hold 5 characters plus a null terminator (at least 6 bytes) [20].
 * @return A pointer to the `base4_buffer` containing the converted string.
 */
char* convert_decimal_to_unique_base4(int decimal_value, char* base4_buffer) {
    /* Machine word size is 10 bits. */
    /* 5 base 4 characters are needed, as each base 4 char represents 2 bits. */
    /* Buffer size must be 5 chars + null terminator = 6 bytes. */
    const char base4_chars[] = {BASE4_A, BASE4_B, BASE4_C, BASE4_D};
    unsigned int i;
    unsigned int temp_value;
    int two_bits;

    /* Use 10-bit unsigned representation to handle 2's complement extraction correctly. */
    /* For example, -5 (int) in 32-bit 2's complement is 0xFFFFFFFB. */
    /* We only care about the lowest 10 bits. */
    /* So, mask to 10 bits: 0xFFFFFFFB & 0x3FF (0x3FF is 1023, or 0b1111111111). */
    temp_value = (unsigned int)decimal_value & 0x3FF; /* Mask to 10 bits */

    /* Convert from right to left (LSB to MSB) */
    for (i = 0; i < 5; i++) {
        /* Extract the last two bits (00, 01, 10, 11) */
        two_bits = temp_value & 0x3; /* 0x3 is 0b11 */
        base4_buffer[4 - i] = base4_chars[two_bits]; /* Store in reverse order in buffer */

        /* Shift right by 2 bits to process the next pair */
        temp_value >>= 2;
    }
    base4_buffer[5] = '\0'; /* Null-terminate the string */

    return base4_buffer;
}

/* --- Opcode and Register Lookup Functions --- */

/**
 * @brief Attempts to retrieve the numerical opcode value for a given operation name string.
 * @param op_name The string representing the assembly operation (e.g., "mov", "add").
 * @param opcode_val A pointer to an integer where the corresponding opcode value will be stored
 *                   if a match is found. This value corresponds to the Opcode enum [23].
 * @return TRUE if the `op_name` matches a defined opcode, FALSE otherwise.
 */
int get_opcode_value(const char* op_name, int* opcode_val) {
    unsigned int i;
    for (i = 0; i < sizeof(opcode_names) / sizeof(opcode_names[0]); i++) {
        if (strcmp(op_name, opcode_names[i]) == 0) {
            *opcode_val = (int)i; /* Opcode enum values match array index */
            return TRUE;
        }
    }
    return FALSE;
}

/**
 * @brief Determines if a given string represents a valid general-purpose register name (r0 to r7) [12, 13].
 * If valid, it also returns the corresponding register number [24].
 * @param str The string to check (e.g., "r0", "r5").
 * @return The integer number of the register (0-7) if `str` is a valid register name,
 *         or -1 if it is not a valid register name.
 */
int get_register_number(const char* str) {
    char digit_char;
    
    /* A valid register name is exactly 2 characters long, starts with 'r', */
    /* and the second character is a digit from '0' to '7'. */
    if (str == NULL || strlen(str) != 2 || str[0] != 'r') {
        return -1; /* Not a valid register format (e.g., "rX") */
    }

    digit_char = str[1]; /* Get the digit character */
    if (digit_char >= '0' && digit_char <= '7') {
        return digit_char - '0'; /* Convert char '0'-'7' to int 0-7 */
    }

    return -1; /* Not a valid register number */
}