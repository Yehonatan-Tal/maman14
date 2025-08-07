#ifndef ASSEMBLER_PRE_ASSEMBLER_H
#define ASSEMBLER_PRE_ASSEMBLER_H

/* Include necessary standard libraries and project definitions */
#include <stdio.h>   /* For FILE* operations */
#include <string.h>  /* For string manipulation functions like strlen, strcmp */

#include "definitions.h" /* Includes global constants like MAX_LINE_LENGTH, MAX_LABEL_LENGTH, etc. */
#include "utils.h"       /* Includes general utility functions like is_legal_label, is_reserved_keyword, etc. */

/**
 * @brief This header file declares functions for the pre-assembler stage of the assembler.
 * Its primary role is to handle the expansion of macros from the initial source file
 * and generate an extended source file (.am file). This module will include the
 * logic for identifying, validating, and expanding macros according to the defined rules.
 */

/* --- Pre-Assembler Core Function --- */

/**
 * @brief Processes a single assembly source file during the pre-assembly stage.
 * This function reads the input .as file, identifies and expands macros,
 * and writes the processed content to an .am file. It also performs
 * initial validation checks specific to macro definitions.
 * @param file_name The base name of the input assembly source file (e.g., "my_program" for "my_program.as").
 * @return TRUE if the pre-assembly stage completed successfully without critical errors
 * that prevent further processing, FALSE otherwise.
 */
int process_pre_assembly_for_file(const char* file_name);

/* --- Macro-Related Utility Functions (for internal use within pre_assembler.c) --- */

/**
 * @brief Checks if a given line marks the beginning of a macro definition.
 * A macro definition starts with the 'mcro' keyword.
 * It also validates the macro name itself (e.g., not a reserved keyword, legal length).
 * @param line The string representing the line to check.
 * @param macro_name_buffer A buffer where the extracted macro name will be stored if valid.
 * @param buffer_size The size of the macro_name_buffer to prevent buffer overflow.
 * @return TRUE if the line is a valid macro definition start, FALSE otherwise.
 */
int is_macro_definition_start(const char* line, char* macro_name_buffer, unsigned int buffer_size);

/**
 * @brief Checks if a given line marks the end of a macro definition.
 * A macro definition ends with the 'mcroend' keyword.
 * @param line The string representing the line to check.
 * @return TRUE if the line is a macro definition end, FALSE otherwise.
 */
int is_macro_definition_end(const char* line);



#endif /* ASSEMBLER_PRE_ASSEMBLER_H */