/*
 * Test program for the Snepper-1 assembler.
 * Counts in a loop, using a function to output.
 */

.0x0000
    jmp     main

output:
    /* Temporary output instruction */
    output  r1
    ret

main:
    /* Set the stack pointer into RAM */
    ldi     hl, 0x2000
    mov     sp, hl

    /* Initialise registers */
    ldi     r1, 0
    ldi     r2, 1

loop:
    add     r1, r2
    call    output
    jmp     loop

