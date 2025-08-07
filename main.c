#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pre_assembler.h"
#include "error_handler.h"

/**
 * @brief Simple main function to test the pre-assembler functionality.
 * Usage: ./assembler <file_name_without_extension>
 * Example: ./assembler tests/valid_macro_example_1
 */
int main(int argc, char* argv[]) {
    const char* file_name;
    int success;
    char am_file_name[256];
    FILE* test_file;
    
    if (argc != 2) {
        printf("Usage: %s <file_name_without_extension>\n", argv[0]);
        printf("Example: %s tests/valid_macro_example_1\n", argv[0]);
        return 1;
    }

    file_name = argv[1];
    
    printf("Starting pre-assembly for file: %s\n", file_name);
    
    /* Process the file through pre-assembler */
    success = process_pre_assembly_for_file(file_name);
    
    if (success) {
        printf("✅ Pre-assembly completed successfully!\n");
        printf("📁 Generated file: %s.am\n", file_name);
        
        /* Check if .am file was created */
        sprintf(am_file_name, "%s.am", file_name);
        test_file = fopen(am_file_name, "r");
        if (test_file) {
            printf("✅ .am file exists and is readable\n");
            fclose(test_file);
        } else {
            printf("❌ .am file was not created or is not readable\n");
            return 1;
        }
    } else {
        printf("❌ Pre-assembly failed!\n");
        if (has_errors()) {
            printf("Errors were detected during processing.\n");
        }
        return 1;
    }
    
    return 0;
} 