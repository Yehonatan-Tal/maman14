; This is a valid assembly file demonstrating macros, instructions, and directives.
; Max line length is 80 characters, as specified in definitions.

.entry MainProgram    ; Declares 'MainProgram' as an entry point.
.extern ExternalData  ; Declares 'ExternalData' as an external symbol.

; Define a macro named 'my_custom_macro'.
; Macro names cannot be reserved keywords like opcodes or directives.
mcro my_custom_macro
    add r3, r4        ; Add operation with two register direct operands.
    prn #-15          ; Print operation with an immediate operand.
    jmp ExitProgram   ; Jump operation with a direct addressing operand.
mcroend

; Data section directives.
; These will populate the data image in memory.
NumbersArray: .data 10, -25, +30, 0    ; Defines an array of integers.
Greeting: .string "Hello Assembler Project!" ; Defines a string.
MatrixData: .mat 1,2,3,4     ; Defines a 2x2 matrix with initial values.

; Instruction section - where the program logic resides.
MainProgram:
    mov #5, r0        ; Move immediate value 5 to register r0.
    inc r0            ; Increment the value in register r0.

    my_custom_macro   ; First call to 'my_custom_macro'.
                      ; This line will be expanded by the pre-assembler.

LoopStart:
    cmp r0, ExternalData ; Compare r0 with an external variable.
    bne LoopStart     ; Branch if not equal, looping back to 'LoopStart'.

    lea NumbersArray, r1 ; Load the effective address of 'NumbersArray'.
    red r2            ; Read a character from stdin into register r2.

    my_custom_macro   ; Second call to 'my_custom_macro'.

    mov MatrixData[r0][r1], r5 ; Move data from 'MatrixData' to r5.
    sub #1, r5        ; Subtract immediate value 1 from register r5.

ExitProgram:
    stop              ; Halts program execution.
                      ; This is an instruction with no operands.

; Another macro call for demonstration
    my_custom_macro