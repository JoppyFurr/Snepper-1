/*
 * Test program for the Snepper-1 assembler.
 * Counts in a loop, using a function to output.
 */

.0x0000
    jmp     main

/*
 * Output r1 using binary-coded decimal
 */
bcd_output:
    push    r1          /* Save modified registers */
    push    r2
    mov     r2, r1      /* r2 contains the remaining value to convert */
    mov     r1, 0       /* r1 contains the bcd for outputting */

bcd_20s_loop:
    cmp     r2, 20
    jmp-neg bcd_10s_loop
    sub     r2, 20
    add     r1, 0x20
    jmp     bcd_10s_loop

bcd_10s_loop:
    cmp     r2, 10
    jmp-neg bcd_finish
    sub     r2, 10
    add     r1, 0x10
    jmp     bcd_10s_loop

bcd_finish:
    add     r1, r2  /* Add the remaining ones */
    output  r1      /* Temporary output instruction */
    pop     r2      /* Restore modified registers */
    pop     r1
    ret

main:
    /* Set the stack pointer into RAM */
    ldi     hl, 0x2000
    mov     sp, hl

    /* Initialise registers */
    mov     r1, 1
    mov     r2, 0
    mov     r4, 0   /* Loop counter */

loop:
    mov     r3, r2
    mov     r2, r1
    add     r1, r3
    call    bcd_output
    add     r4, 1
    cmp     r4, 10
    jmp-z   exit
    jmp     loop

exit:
    halt

