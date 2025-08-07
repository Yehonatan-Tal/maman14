# Makefile for Assembler Project
CC = gcc
CFLAGS = -Wall -ansi -pedantic -std=c99
TARGET = assembler

# Source files
SOURCES = main.c pre_assembler.c utils.c error_handler.c
OBJECTS = $(SOURCES:.c=.o)

# Header files
HEADERS = pre_assembler.h utils.h error_handler.h definitions.h

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS)

# Compile source files
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -f $(OBJECTS) $(TARGET)

# Test the pre-assembler
test: $(TARGET)
	@echo "Testing pre-assembler with valid_macro_example_1.as..."
	./$(TARGET) tests/valid_macro_example_1

# Show the generated .am file
show-am: test
	@echo ""
	@echo "Generated .am file content:"
	@echo "=========================="
	@cat tests/valid_macro_example_1.am

# Run with invalid file to test error handling
test-invalid: $(TARGET)
	@echo "Testing pre-assembler with invalid_assembly_example_1.as..."
	./$(TARGET) tests/invalid_assembly_example_1

.PHONY: all clean test show-am test-invalid 