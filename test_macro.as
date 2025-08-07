; Test macro file
mcro MyMacro
    mov r1, r2
    add r3, r4
mcroend

; Some code before
mov r5, r6

; Call the macro
MyMacro

; Some code after
add r7, r8 