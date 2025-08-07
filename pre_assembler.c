#include "pre_assembler.h"
#include "error_handler.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* --- Macro Structure Definition --- */

/**
 * @brief Structure to represent a macro definition
 */
typedef struct {
    char name[MAX_LABEL_LENGTH];  /* Macro name */
    char* body;                       /* Macro body content */
    int body_length;                  /* Length of macro body */
} Macro;

/* --- Global Variables for Macro Management --- */

static Macro* g_macros = NULL;        /* Array of defined macros */
static int g_macro_count = 0;         /* Number of defined macros */
static int g_macro_capacity = 0;      /* Capacity of macros array */

/* --- Internal Helper Functions --- */

/**
 * @brief Adds a new macro to the global macros array
 * @param name The name of the macro
 * @param body The body content of the macro
 * @param body_length The length of the macro body
 */
static void add_macro(const char* name, const char* body, int body_length) {
    /* Expand array if needed */
    if (g_macro_count >= g_macro_capacity) {
        int new_capacity = (g_macro_capacity == 0) ? 10 : g_macro_capacity * 2;
        Macro* new_macros = (Macro*)realloc(g_macros, new_capacity * sizeof(Macro));
        g_macros = new_macros;
        g_macro_capacity = new_capacity;
    }
    
    /* Copy macro name */
    strncpy(g_macros[g_macro_count].name, name, MAX_LABEL_LENGTH);
    g_macros[g_macro_count].name[MAX_LABEL_LENGTH] = '\0';
    
    /* Allocate and copy macro body */
    g_macros[g_macro_count].body = (char*)malloc(body_length + 1);
    strncpy(g_macros[g_macro_count].body, body, body_length);
    g_macros[g_macro_count].body[body_length] = '\0';
    g_macros[g_macro_count].body_length = body_length;
    
    g_macro_count++;
}

/**
 * @brief Finds a macro by name
 * @param name The name of the macro to find
 * @return Pointer to the macro if found, NULL otherwise
 */
static Macro* find_macro(const char* name) {
    int i;
    for (i = 0; i < g_macro_count; i++) {
        if (strcmp(g_macros[i].name, name) == 0) {
            return &g_macros[i];
        }
    }
    return NULL;
}

/**
 * @brief Frees all allocated macro memory
 */
static void free_macros() {
    int i;
    for (i = 0; i < g_macro_count; i++) {
        if (g_macros[i].body != NULL) {
            free(g_macros[i].body);
        }
    }
    if (g_macros != NULL) {
        free(g_macros);
    }
    g_macros = NULL;
    g_macro_count = 0;
    g_macro_capacity = 0;
}

/**
 * @brief Validates macro name according to the rules
 * @param name The macro name to validate
 * @return TRUE if valid, FALSE otherwise
 */
static int is_valid_macro_name(const char* name) {
    int len;
    const char* original_name = name;
    
    /* Check if name is NULL or empty */
    if (name == NULL || *name == '\0') {
        return FALSE;
    }
    
    /* Check length */
    len = strlen(name);
    if (len > MAX_LABEL_LENGTH) {
        return FALSE;
    }
    
    /* Check if starts with underscore */
    if (name[0] == '_') {
        return FALSE;
    }
    
    /* Check if starts with letter */
    if (!is_alpha(name[0])) {
        return FALSE;
    }
    
    /* Check all characters are valid */
    while (*name != '\0') {
        if (!is_alphanumeric(*name)) {
            return FALSE;
        }
        name++;
    }
    
    /* Check if it's a reserved keyword */
    if (is_reserved_keyword(original_name)) {
        return FALSE;
    }
    
    return TRUE;
}

/**
 * @brief Extracts macro name from a line starting with "mcro"
 * @param line The line to parse
 * @param macro_name Buffer to store the macro name
 * @param buffer_size Size of the buffer
 * @return TRUE if valid macro definition, FALSE otherwise
 */
static int extract_macro_name(const char* line, char* macro_name, unsigned int buffer_size) {
    char* trimmed_line;
    char* token;
    char line_copy[MAX_LINE_LENGTH + 1];
    char extracted_name[MAX_LABEL_LENGTH + 1];
    
    /* Copy line to avoid modifying original */
    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH] = '\0';
    
    /* Trim whitespace */
    trimmed_line = trim_whitespace(line_copy);
    
    /* Skip "mcro" keyword */
    if (strncmp(trimmed_line, "mcro", 4) != 0) {
        return FALSE;
    }
    
    /* Skip "mcro" and any whitespace */
    trimmed_line += 4;
    trimmed_line = skip_whitespace(trimmed_line);
    
    /* Extract macro name */
    token = trimmed_line;
    while (*token != '\0' && !is_whitespace(*token)) {
        token++;
    }
    
    /* Check if we have a macro name */
    if (token == trimmed_line) {
        return FALSE;
    }
    
    /* Copy macro name to temporary buffer */
    if ((unsigned int)(token - trimmed_line) >= sizeof(extracted_name)) {
        return FALSE;
    }
    
    strncpy(extracted_name, trimmed_line, token - trimmed_line);
    extracted_name[token - trimmed_line] = '\0';
    
    /* Check if there are any additional tokens */
    token = skip_whitespace(token);
    if (*token != '\0' && *token != ';') {
        return FALSE;
    }
    
    /* Copy to output buffer */
    if (strlen(extracted_name) >= (int)buffer_size) {
        return FALSE;
    }
    
    strcpy(macro_name, extracted_name);
    return TRUE;
}

/**
 * @brief Checks if a line is a macro call
 * @param line The line to check
 * @return Name of macro if it's a call, NULL otherwise
 */
static const char* is_macro_call(const char* line) {
    char* trimmed_line;
    char line_copy[MAX_LINE_LENGTH + 1];
    char macro_name[MAX_LABEL_LENGTH + 1];
    char* token;
    
    /* Copy line to avoid modifying original */
    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH] = '\0';
    
    /* Trim whitespace */
    trimmed_line = trim_whitespace(line_copy);
    
    /* Skip label if present */
    if (strchr(trimmed_line, ':') != NULL) {
        trimmed_line = strchr(trimmed_line, ':') + 1;
        trimmed_line = skip_whitespace(trimmed_line);
    }
    
    /* Extract first token */
    token = trimmed_line;
    while (*token != '\0' && !is_whitespace(*token)) {
        token++;
    }
    
    /* Copy token */
    if (token == trimmed_line) {
        return NULL;
    }
    
    if ((unsigned int)(token - trimmed_line) > MAX_LABEL_LENGTH) {
        return NULL;
    }
    
    strncpy(macro_name, trimmed_line, token - trimmed_line);
    macro_name[token - trimmed_line] = '\0';
    
    /* Check if it's a macro name */
    {
        Macro* macro = find_macro(macro_name);
        return macro ? macro->name : NULL;
    }
}

/* --- Public Functions Implementation --- */

int is_macro_definition_start(const char* line, char* macro_name_buffer, unsigned int buffer_size) {
    if (line == NULL || macro_name_buffer == NULL) {
        return FALSE;
    }
    
    return extract_macro_name(line, macro_name_buffer, buffer_size);
}

int is_macro_definition_end(const char* line) {
    char* trimmed_line;
    char line_copy[MAX_LINE_LENGTH + 1];
    
    if (line == NULL) {
        return FALSE;
    }
    
    /* Copy line to avoid modifying original */
    strncpy(line_copy, line, MAX_LINE_LENGTH);
    line_copy[MAX_LINE_LENGTH] = '\0';
    
    /* Trim whitespace */
    trimmed_line = trim_whitespace(line_copy);
    
    /* Check for "mcroend" */
    if (strncmp(trimmed_line, "mcroend", 7) == 0) {
        /* Check if there are additional tokens */
        trimmed_line += 7;
        trimmed_line = skip_whitespace(trimmed_line);
        return (*trimmed_line == '\0' || *trimmed_line == ';');
    }
    
    return FALSE;
}

int process_pre_assembly_for_file(const char* file_name) {
    FILE* input_file = NULL;
    FILE* output_file = NULL;
    char input_filename[256];
    char output_filename[256];
    char line[MAX_LINE_LENGTH + 2];  /* +2 for newline and null terminator */
    int line_number = 0;
    int in_macro_definition = FALSE;
    char current_macro_name[MAX_LABEL_LENGTH + 1];
    char* macro_body = NULL;
    int macro_body_size = 0;
    int macro_body_capacity = 0;
    int has_errors_in_file = FALSE;
    
    /* Reset error flag */
    reset_error_flag();
    
    /* Free any existing macros */
    free_macros();
    
    /* Construct filenames */
    sprintf(input_filename, "%s%s", file_name, AS_EXTENSION);
    sprintf(output_filename, "%s%s", file_name, AM_EXTENSION);
    
    /* Open input file */
    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
        return FALSE;
    }
    
    /* First pass: collect macro definitions */
    while (fgets(line, sizeof(line), input_file) != NULL) {
        line_number++;
        
        /* Check line length */
        if (strlen(line) > MAX_LINE_LENGTH + 1) {  /* +1 for newline */
            report_error(file_name, line_number, ERROR_LINE_TOO_LONG);
            has_errors_in_file = TRUE;
            continue;
        }
        
        /* Check for macro definition start */
        if (is_macro_definition_start(line, current_macro_name, sizeof(current_macro_name))) {
            /* Validate macro name */
            if (!is_valid_macro_name(current_macro_name)) {
                if (is_reserved_keyword(current_macro_name)) {
                    report_error(file_name, line_number, ERROR_MACRO_NAME_RESERVED_KEYWORD);
                } else {
                    report_error(file_name, line_number, ERROR_MACRO_NAME_INVALID_FORMAT);
                }
                has_errors_in_file = TRUE;
                continue;
            }
            
            /* Check for duplicate macro name */
            if (find_macro(current_macro_name) != NULL) {
                report_error(file_name, line_number, ERROR_LABEL_REDEFINITION);
                has_errors_in_file = TRUE;
                continue;
            }
            
            in_macro_definition = TRUE;
            macro_body_size = 0;
            continue;
        }
        
        /* Check for macro definition end */
        if (is_macro_definition_end(line)) {
            /* Add macro to collection */
            if (macro_body_size > 0) {
                add_macro(current_macro_name, macro_body, macro_body_size);
            }
            
            in_macro_definition = FALSE;
            if (macro_body != NULL) {
                free(macro_body);
                macro_body = NULL;
            }
            macro_body_size = 0;
            macro_body_capacity = 0;
            continue;
        }
        
        /* If in macro definition, collect body */
        if (in_macro_definition) {
            int line_len = strlen(line);
            
            /* Expand macro body buffer if needed */
            if (macro_body_size + line_len >= macro_body_capacity) {
                int new_capacity = (macro_body_capacity == 0) ? 1024 : macro_body_capacity * 2;
                char* new_body = (char*)realloc(macro_body, new_capacity);
                macro_body = new_body;
                macro_body_capacity = new_capacity;
            }
            
            /* Add line to macro body */
            strcpy(macro_body + macro_body_size, line);
            macro_body_size += line_len;
        }
    }

    /* Free macro body if still allocated */
    if (macro_body != NULL) {
        free(macro_body);
    }
    
    /* Close input file */
    fclose(input_file);
    
    /* Only create output file if no errors were found */
    if (!has_errors_in_file) {
        output_file = fopen(output_filename, "w");
        if (output_file == NULL) {
            fclose(input_file);
            report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
            return FALSE;
        }
    }
    
    /* Second pass: expand macros and write output */
    input_file = fopen(input_filename, "r");
    if (input_file == NULL) {
        if (output_file != NULL) {
            fclose(output_file);
        }
        report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
        return FALSE;
    }
    
    line_number = 0;
    in_macro_definition = FALSE;
    
    while (fgets(line, sizeof(line), input_file) != NULL) {
        line_number++;
        
        /* Check for macro definition start */
        if (is_macro_definition_start(line, current_macro_name, sizeof(current_macro_name))) {
            in_macro_definition = TRUE;
            continue;
        }
        
        /* Check for macro definition end */
        if (is_macro_definition_end(line)) {
            in_macro_definition = FALSE;
            continue;
        }
        
        /* Skip lines that are part of macro definition */
        if (in_macro_definition) {
            continue;
        }
        
        /* Check if line is a macro call */
        if (output_file != NULL) {
            const char* macro_name = is_macro_call(line);
            if (macro_name != NULL) {
                Macro* called_macro = find_macro(macro_name);
                if (called_macro != NULL) {
                    /* Check if line has a label */
                    char* colon_pos = strchr(line, ':');
                    if (colon_pos != NULL) {
                        /* Write label part */
                        int label_len = colon_pos - line + 1;
                        fwrite(line, 1, label_len, output_file);
                        /* Write macro body */
                        fputs(called_macro->body, output_file);
                    } else {
                        /* Write macro body to output */
                        fputs(called_macro->body, output_file);
                    }
                }
            } else {
                /* Write original line to output */
                fputs(line, output_file);
            }
        }
    }

    /* Close files */
    fclose(input_file);
    if (output_file != NULL) {
        fclose(output_file);
    }
    
    /* Free macros */
    free_macros();
    
    /* Return success if no errors occurred */
    return !has_errors_in_file && !has_errors();
}
