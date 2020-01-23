/*
 * Test program for the Snepper-1 assembler.
 * Counts in a loop, using a function to output.
 */

.0x0000
    jmp     main

/*
 * Output a number using binary-coded decimal
 */
bcd_output:
    push    r1          /* Save r1 and r2 */
    push    r2
    mov     r2, r1      /* r2 contains the remaining value to convert */
    ldi     r1, 0       /* r1 contains the bcd for outputting */

bcd_10s_loop:
    ldi     r4, 10      /* if r2 < 10 */
    cmp     r2, r4

    jmp-neg bcd_10s_done    /* No more tens to convert */

    ldi     r3, 10      /* Subtract 10 from r2 */
    sub     r2, r3
    ldi     r3, 0x10    /* Add 0x10 to r1 */
    add     r1, r3

    jmp     bcd_10s_loop

bcd_10s_done:
    add     r1, r2  /* Add the remaining ones */
    output  r1      /* Temporary output instruction */
    pop     r2      /* Restore r1 and r2 */
    pop     r1
    ret


main:
    /* Set the stack pointer into RAM */
    ldi     hl, 0x2000
    mov     sp, hl

    /* Initialise registers */
    ldi     r1, 1
    ldi     r2, 0

loop:
    mov     r3, r2
    mov     r2, r1
    add     r1, r3
    call    bcd_output
    jmp     loop

