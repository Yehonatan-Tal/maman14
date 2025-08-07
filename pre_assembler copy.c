#include <stdio.h>
#include <stdlib.h> /* For malloc, free, exit  */
#include <string.h> /* For strlen, strcmp, strcpy, strncpy, strtok  */
#include <ctype.h>  /* For isspace */

#include "pre_assembler.h"
#include "definitions.h"
#include "utils.h"
#include "error_handler.h"

/* --- Internal Data Structures for Macro Management --- */

/**
 * @brief Represents a single line of content within a macro definition.
 */
typedef struct MacroContentNode {
    char* line_text; /* Dynamically allocated string for the line content */
    struct MacroContentNode* next;
} MacroContentNode;

/**
 * @brief Represents a defined macro, including its name and a linked list of its content lines.
 */
typedef struct Macro {
    char name[MAX_LABEL_LENGTH + 1]; /* Name of the macro */
    MacroContentNode* content_head;   /* Head of the linked list of content lines */
    MacroContentNode* content_tail;   /* Tail of the linked list for efficient appending */
} Macro;

/**
 * @brief Node in the linked list of defined macros.
 */
typedef struct MacroListNode {
    Macro macro;
    struct MacroListNode* next;
} MacroListNode;

/* --- Static Global Variables (Internal to this module) --- */

/* Head of the global linked list of all defined macros */
static MacroListNode* g_macros_list_head = NULL;

/* Flag to track if currently inside a macro definition block */
static int g_in_macro_definition = FALSE;

/* Pointer to the macro currently being defined */
static Macro* g_current_macro = NULL;

/* Counter for the current line number being processed in the source file */
static int g_current_line_number = 0;

/*
 * @brief Placeholder for file_name to be used with report_error.
 * The actual file name is passed to process_pre_assembly_for_file.
 * In a more complex setup, this would be managed more robustly (e.g., via context struct).
 * For this assignment, assuming a single file is processed at a time.
 */
static const char* s_current_file_name_for_errors = NULL;

/* --- Internal Helper Function Prototypes --- */

static MacroListNode* create_macro_list_node(const char* macro_name);
static void add_macro_to_list(MacroListNode* new_macro_node);
static Macro* find_macro_in_list(const char* macro_name);
static MacroContentNode* create_macro_content_node(const char* line_text);
static int add_line_to_current_macro_content(const char* line_text);
static void free_all_macros();
static void copy_macro_content_to_file(FILE* output_file, const Macro* macro_to_expand);

/*
 * @brief This helper is used to pass the current file name to error_handler,
 * as report_error expects it and the main file name isn't a global in error_handler.
 */
const char* file_name_placeholder(void);

/*
 * @brief Initializes the s_current_file_name_for_errors. Should be called by main.
 * (Not part of the specific request, but necessary for report_error to work correctly).
 */
void set_file_name_for_errors(const char* name);

/* --- Pre-Assembler Core Function Implementation --- */

/**
 * @brief Processes a single assembly source file during the pre-assembly stage.
 * This function reads the input .as file, identifies and expands macros,
 * and writes the processed content to an .am file. It also performs
 * initial validation checks specific to macro definitions.
 * @param file_name The base name of the input assembly source file (e.g., "my_program" for "my_program.as").
 * @return TRUE if the pre-assembly stage completed successfully without critical errors
 * that prevent further processing, FALSE otherwise.
 */
int process_pre_assembly_for_file(const char* file_name) {
    FILE* as_file = NULL;
    FILE* am_file = NULL;
    char input_file_name[MAX_LINE_LENGTH + AS_EXTENSION_LENGTH + 1]; /* Includes .as extension */
    char output_file_name[MAX_LINE_LENGTH + AM_EXTENSION_LENGTH + 1]; /* Includes .am extension */
    char line_buffer[MAX_LINE_LENGTH + 2]; /* Plus 2 for newline and null terminator */
    char trimmed_line[MAX_LINE_LENGTH + 1]; /* For trimmed line copy */
    char temp_macro_name[MAX_LABEL_LENGTH + 1]; /* Buffer for extracted macro name */
    int overall_success = TRUE;
    const char* called_macro_name = NULL;
    Macro* called_macro = NULL;
    char line_copy_for_call_check[MAX_LINE_LENGTH + 1];
    char* first_token = NULL;
    MacroListNode* new_node = NULL;
    int has_non_empty_lines = FALSE; /* Track if file has any non-empty, non-comment lines */

    g_current_line_number = 0; /* Reset line counter for the new file */
    g_in_macro_definition = FALSE;
    g_current_macro = NULL;
    free_all_macros(); /* Clear any previously defined macros */
    reset_error_flag(); /* Reset error flag for the new file */
    s_current_file_name_for_errors = file_name; /* Set file name for error reporting */

    /* Construct file names */
    sprintf(input_file_name, "%s%s", file_name, AS_EXTENSION);
    sprintf(output_file_name, "%s%s", file_name, AM_EXTENSION);

    /* Open input file */
    as_file = fopen(input_file_name, "r");
    if (!as_file) {
        report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
        return FALSE;
    }

    /* Don't open output file yet - we'll open it only if no errors occur */
    am_file = NULL;

    /* Process the file line by line */
    
    while (fgets(line_buffer, sizeof(line_buffer), as_file) != NULL) {
        g_current_line_number++;

        /* Check for line length limit */
        if (strlen(line_buffer) > MAX_LINE_LENGTH && line_buffer[MAX_LINE_LENGTH] != '\n' && line_buffer[MAX_LINE_LENGTH] != '\0') {
            report_error(file_name, g_current_line_number, ERROR_LINE_TOO_LONG);
            /* Continue processing to find more errors, but don't write this line or process its content further */
            continue;
        }

        /* Create a modifiable copy of the line for trimming and tokenizing */
        strncpy(trimmed_line, line_buffer, MAX_LINE_LENGTH);
        trimmed_line[MAX_LINE_LENGTH] = '\0'; /* Ensure null termination */
        trim_whitespace(trimmed_line); /* Trim the copy */
        
        /* Check for unexpected 'mcroend' without 'mcro' */
        if (!g_in_macro_definition && is_macro_definition_end(trimmed_line)) {
            report_error(file_name, g_current_line_number, ERROR_UNEXPECTED_MACRO_END);
            /* Continue processing to find more errors */
        }

        /* Skip empty or comment lines if not inside a macro definition (comments inside macros are part of macro content) */
        if (!g_in_macro_definition && is_empty_or_comment_line(trimmed_line)) {
            if (!has_errors()) {
                /* Open output file if not already open */
                if (am_file == NULL) {
                    am_file = fopen(output_file_name, "w");
                    if (!am_file) {
                        report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
                        fclose(as_file);
                        return FALSE;
                    }
                }
                fputs(line_buffer, am_file); /* Copy comments/empty lines to .am file */
            }
            continue;
        } else if (!g_in_macro_definition) {
            /* This is a non-empty, non-comment line */
            has_non_empty_lines = TRUE;
        }

        /* Check if inside a macro definition */
        if (g_in_macro_definition) {
            if (is_macro_definition_end(trimmed_line)) {
                /* End of macro definition */
                g_in_macro_definition = FALSE;
                g_current_macro = NULL;
            } else {
                /* Add line to current macro's content */
                if (!add_line_to_current_macro_content(line_buffer)) {
                    /* Memory allocation failure or similar, critical error */
                    overall_success = FALSE;
                    /* Continue processing to find more errors */
                }
            }
        } else {
            /* Not in macro definition - check for macro call first, then macro start */
            
            /* Check for macro call first */
            called_macro_name = NULL;
            called_macro = NULL;

            /* Since is_macro_call uses strtok, it needs a mutable string. Use a copy. */
            strncpy(line_copy_for_call_check, trimmed_line, sizeof(line_copy_for_call_check) - 1);
            line_copy_for_call_check[sizeof(line_copy_for_call_check) - 1] = '\0';

            /* Extract potential macro name (first word) */
            first_token = strtok(line_copy_for_call_check, " \t\n\r");
            if (first_token) {
                called_macro_name = first_token;
                called_macro = find_macro_in_list(called_macro_name);
                
                /* If it's a macro, check for extra characters (ignore comments) */
                if (called_macro) {
                    char* extra_token = strtok(NULL, " \t\n\r");
                    if (extra_token != NULL) {
                        /* Check if the extra token is a comment (starts with ';') */
                        if (extra_token[0] == ';') {
                            /* It's a comment, which is allowed */
                        } else {
                            /* This is a syntax error, a macro call should only be the macro name on the line */
                            report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_GENERIC_SYNTAX_ERROR);
                            called_macro = NULL; /* Don't expand this macro due to syntax error */
                        }
                    }
                }
            }

            if (called_macro) {
                /* Expand macro */
                if (!has_errors()) {
                    /* Open output file if not already open */
                    if (am_file == NULL) {
                        am_file = fopen(output_file_name, "w");
                        if (!am_file) {
                            report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
                            fclose(as_file);
                            return FALSE;
                        }
                    }
                    copy_macro_content_to_file(am_file, called_macro);
                }
            } else {
                /* Not a macro call - check if it's a macro start */
                if (is_macro_definition_start(trimmed_line, temp_macro_name, sizeof(temp_macro_name))) {
                    /* Start of new macro definition */
                    new_node = create_macro_list_node(temp_macro_name);
                    if (!new_node) {
                        /* Memory allocation failure */
                        overall_success = FALSE;
                        /* Continue processing to find more errors */
                    } else {
                        add_macro_to_list(new_node);
                        g_current_macro = &(new_node->macro);
                        g_in_macro_definition = TRUE;
                    }
                } else {
                    /* Not a macro start, not a macro end, not a macro call - copy original line */
                    if (!has_errors()) {
                        /* Open output file if not already open */
                        if (am_file == NULL) {
                            am_file = fopen(output_file_name, "w");
                            if (!am_file) {
                                report_error(file_name, 0, ERROR_FILE_OPEN_FAILED);
                                fclose(as_file);
                                return FALSE;
                            }
                        }
                        fputs(line_buffer, am_file);
                    }
                }
            }
        }
    }

    /* After file processing, check for unclosed macro definition */
    if (g_in_macro_definition) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_UNCLOSED_MACRO_DEFINITION);
        overall_success = FALSE;
    }
    
    /* Check if file is empty or contains only comments */
    if (!has_non_empty_lines) {
        report_error(file_name, 0, ERROR_EMPTY_OR_COMMENT_FILE);
        overall_success = FALSE;
    }

    /* Final cleanup */
    fclose(as_file);
    
    /* If there were errors, close the .am file if it was opened and delete it */
    if (has_errors() || !overall_success) {
        if (am_file != NULL) {
            fclose(am_file);
            /* Delete the .am file since it contains errors */
            if (remove(output_file_name) != 0) {
                printf("Warning: Could not delete .am file: %s\n", output_file_name);
            } else {
                printf("Deleted .am file due to errors: %s\n", output_file_name);
            }
        }
        overall_success = FALSE;
    } else {
        /* Close the .am file if it was opened */
        if (am_file != NULL) {
            fclose(am_file);
        }
    }
    
    free_all_macros();

    /* Print summary of errors found */
    if (has_errors()) {
        printf("Pre-assembly completed with errors. No .am file generated.\n");
    }

    return overall_success;
}

/* --- Macro-Related Utility Functions Implementation (for internal use within pre_assembler.c) --- */

/**
 * @brief Checks if a given line marks the beginning of a macro definition.
 * A macro definition starts with the 'mcro' keyword.
 * It also validates the macro name itself (e.g., not a reserved keyword, legal length).
 * @param line The string representing the line to check.
 * @param macro_name_buffer A buffer where the extracted macro name will be stored if valid.
 * @param buffer_size The size of the macro_name_buffer to prevent buffer overflow.
 * @return TRUE if the line is a valid macro definition start, FALSE otherwise.
 */
int is_macro_definition_start(const char* line, char* macro_name_buffer, size_t buffer_size) {
    char line_copy[MAX_LINE_LENGTH + 1];
    char* macro_keyword;
    char* macro_name;
    const char* after_macro_name_ptr;
    size_t macro_name_len;

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    macro_keyword = strtok(line_copy, " \t\n\r");

    if (macro_keyword == NULL || strcmp(macro_keyword, "mcro") != 0) {
        return FALSE; /* Not a 'mcro' keyword */
    }

    macro_name = strtok(NULL, " \t\n\r");
    if (macro_name == NULL) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_MACRO_DEFINITION_SYNTAX);
        return FALSE; /* Missing macro name */
    }

    // מצא את המיקום של שם המאקרו בשורה המקורית
    const char* macro_name_pos = strstr(line, macro_name);
    if (!macro_name_pos) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_INTERNAL_ERROR);
        return FALSE;
    }
    after_macro_name_ptr = macro_name_pos + strlen(macro_name);

    // דלג על רווחים/טאבים
    while (*after_macro_name_ptr == ' ' || *after_macro_name_ptr == '\t') {
        after_macro_name_ptr++;
    }
    // אם הגענו לסוף השורה או להערה, זה תקין
    if (*after_macro_name_ptr == '\0' || *after_macro_name_ptr == '\n' || *after_macro_name_ptr == ';') {
        // תקין
    } else {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_MACRO_DEFINITION_SYNTAX);
        return FALSE;
    }

    // בדוק חוקיות שם המאקרו
    if (!is_legal_label(macro_name)) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_MACRO_NAME_INVALID_FORMAT);
        return FALSE;
    }
    if (is_reserved_keyword(macro_name)) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_MACRO_NAME_RESERVED_KEYWORD);
        return FALSE;
    }

    // בדוק שאין קינון מאקרו
    if (g_in_macro_definition) {
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_NESTED_MACRO_DEFINITION);
        return FALSE;
    }

    strncpy(macro_name_buffer, macro_name, buffer_size - 1);
    macro_name_buffer[buffer_size - 1] = '\0';
    return TRUE;
}

/**
 * @brief Checks if a given line marks the end of a macro definition.
 * A macro definition ends with the 'mcroend' keyword.
 * @param line The string representing the line to check.
 * @return TRUE if the line is a macro definition end, FALSE otherwise.
 */
int is_macro_definition_end(const char* line) {
    /* Use a copy of the line as strtok modifies it */
    char line_copy[MAX_LINE_LENGTH + 1];
    char* token;

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    token = strtok(line_copy, " \t\n\r");

    if (token == NULL || strcmp(token, "mcroend") != 0) {
        return FALSE;
    }

    /* Check if there are any extra characters after 'mcroend' (ignore comments) */
    token = strtok(NULL, " \t\n\r");
    if (token != NULL) {
        /* Check if the extra token is a comment (starts with ';') */
        if (token[0] == ';') {
            /* It's a comment, which is allowed */
        } else {
            report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_MACRO_DEFINITION_SYNTAX); /* Reusing syntax error type */
            return FALSE;
        }
    }

    return TRUE;
}

/**
 * @brief Determines if a given line is a call to a previously defined macro.
 * This function checks against the global list of defined macros.
 * @param line The string representing the line to check.
 * @param macro_names_list_ptr Unused parameter kept for interface compatibility.
 * @return The name of the macro if the line is a macro call, or NULL if not.
 */
const char* is_macro_call(const char* line, void* macro_names_list_ptr) {
    /* Note: macro_names_list_ptr is unused but kept for interface compatibility */
    /* We use the static global g_macros_list_head directly */

    char line_copy[MAX_LINE_LENGTH + 1];
    char* first_token;
    Macro* found_macro;
    char* extra_token;

    strncpy(line_copy, line, sizeof(line_copy) - 1);
    line_copy[sizeof(line_copy) - 1] = '\0';

    first_token = strtok(line_copy, " \t\n\r");

    if (first_token == NULL) {
        return NULL; /* Empty line or just whitespace */
    }

    /* Check if the first token is a defined macro name */
    found_macro = find_macro_in_list(first_token);
    if (found_macro != NULL) {
        /* Check if there are any extra characters after the macro call (ignore comments) */
        extra_token = strtok(NULL, " \t\n\r");
        if (extra_token != NULL) {
            /* Check if the extra token is a comment (starts with ';') */
            if (extra_token[0] == ';') {
                /* It's a comment, which is allowed */
            } else {
                /* This is a syntax error, a macro call should only be the macro name on the line */
                report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_GENERIC_SYNTAX_ERROR);
                return NULL;
            }
        }
        return found_macro->name; /* Return the actual name from the stored macro */
    }

    return NULL;
}


/* --- Internal Helper Function Implementations --- */

/**
 * @brief Creates a new MacroListNode and initializes the Macro struct within it.
 * @param macro_name The name of the macro.
 * @return A pointer to the newly created node, or NULL if memory allocation fails.
 */
static MacroListNode* create_macro_list_node(const char* macro_name) {
    MacroListNode* new_node = (MacroListNode*)malloc(sizeof(MacroListNode));
    if (new_node == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for macro list node.\n");
        return NULL;
    }
    strncpy(new_node->macro.name, macro_name, MAX_LABEL_LENGTH);
    new_node->macro.name[MAX_LABEL_LENGTH] = '\0';
    new_node->macro.content_head = NULL;
    new_node->macro.content_tail = NULL;
    new_node->next = NULL;
    return new_node;
}

/**
 * @brief Adds a new macro node to the global list of macros.
 * @param new_macro_node A pointer to the MacroListNode to add.
 */
static void add_macro_to_list(MacroListNode* new_macro_node) {
    MacroListNode* current;
    
    if (g_macros_list_head == NULL) {
        g_macros_list_head = new_macro_node;
    } else {
        current = g_macros_list_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_macro_node;
    }
}

/**
 * @brief Finds a macro by name in the global list of macros.
 * @param macro_name The name of the macro to find.
 * @return A pointer to the Macro struct if found, otherwise NULL.
 */
static Macro* find_macro_in_list(const char* macro_name) {
    MacroListNode* current = g_macros_list_head;
    while (current != NULL) {
        if (strcmp(current->macro.name, macro_name) == 0) {
            return &(current->macro);
        }
        current = current->next;
    }
    return NULL;
}

/**
 * @brief Creates a new MacroContentNode.
 * @param line_text The text content of the line.
 * @return A pointer to the newly created node, or NULL if memory allocation fails.
 */
static MacroContentNode* create_macro_content_node(const char* line_text) {
    MacroContentNode* new_node = (MacroContentNode*)malloc(sizeof(MacroContentNode));
    if (new_node == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for macro content node.\n");
        return NULL;
    }
    new_node->line_text = (char*)malloc(strlen(line_text) + 1);
    if (new_node->line_text == NULL) {
        fprintf(stderr, "Fatal Error: Memory allocation failed for macro content line text.\n");
        free(new_node);
        return NULL;
    }
    strcpy(new_node->line_text, line_text);
    new_node->next = NULL;
    return new_node;
}

/**
 * @brief Adds a line of text to the content of the macro currently being defined.
 * @param line_text The line to add.
 * @return TRUE if successful, FALSE if memory allocation fails.
 */
static int add_line_to_current_macro_content(const char* line_text) {
    MacroContentNode* new_content_node;
    
    if (g_current_macro == NULL) {
        /* This should not happen if logic is correct, but as a safeguard */
        report_error(s_current_file_name_for_errors, g_current_line_number, ERROR_INTERNAL_ERROR);
        return FALSE;
    }

    new_content_node = create_macro_content_node(line_text);
    if (new_content_node == NULL) {
        return FALSE; /* Memory allocation failure handled in create_macro_content_node */
    }

    if (g_current_macro->content_head == NULL) {
        g_current_macro->content_head = new_content_node;
        g_current_macro->content_tail = new_content_node;
    } else {
        g_current_macro->content_tail->next = new_content_node;
        g_current_macro->content_tail = new_content_node;
    }
    return TRUE;
}

/**
 * @brief Frees all dynamically allocated memory for macros and their content.
 */
static void free_all_macros() {
    MacroListNode* current_macro_node = g_macros_list_head;
    MacroListNode* temp_macro_node;
    MacroContentNode* current_content_node;
    MacroContentNode* temp_content_node;
    
    while (current_macro_node != NULL) {
        current_content_node = current_macro_node->macro.content_head;
        while (current_content_node != NULL) {
            temp_content_node = current_content_node;
            current_content_node = current_content_node->next;
            free(temp_content_node->line_text);
            free(temp_content_node);
        }
        temp_macro_node = current_macro_node;
        current_macro_node = current_macro_node->next;
        free(temp_macro_node);
    }
    g_macros_list_head = NULL;
    g_current_macro = NULL; /* Ensure current_macro is also reset */
}

/**
 * @brief Copies the content lines of a given macro to the output file.
 * @param output_file The file pointer to write to.
 * @param macro_to_expand A pointer to the Macro struct whose content is to be copied.
 */
static void copy_macro_content_to_file(FILE* output_file, const Macro* macro_to_expand) {
    MacroContentNode* current_content_node = macro_to_expand->content_head;
    while (current_content_node != NULL) {
        fputs(current_content_node->line_text, output_file);
        current_content_node = current_content_node->next;
    }
}

/*
 * @brief This helper is used to pass the current file name to error_handler,
 * as report_error expects it and the main file name isn't a global in error_handler.
 */
const char* file_name_placeholder() {
    return s_current_file_name_for_errors;
}

/*
 * @brief Initializes the s_current_file_name_for_errors. Should be called by main.
 * (Not part of the specific request, but necessary for report_error to work correctly).
 */
void set_file_name_for_errors(const char* name) {
    s_current_file_name_for_errors = name;
}