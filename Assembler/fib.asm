/*
 * Test program for the Snepper-1 assembler.
 * Counts in a loop, using a function to output.
 */

.0x0000
    jmp     main

.0x0010
str_snepper1:
    data    "SNEPPER1"

.0x1000

/*
 * Display the string stored at 0x4000
 */
show_string:
    push    r3          /* Save modified registers */

    /* Perhaps cfg-set rX would be nice... */
    cfg-clr 0xf0
    cfg-set 0x00
    ld      r3, [0x4000]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x01
    ld      r3, [0x4001]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x02
    ld      r3, [0x4002]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x03
    ld      r3, [0x4003]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x04
    ld      r3, [0x4004]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x05
    ld      r3, [0x4005]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x06
    ld      r3, [0x4006]
    output  r3

    cfg-clr 0xf0
    cfg-set 0x07
    ld      r3, [0x4007]
    output  r3

    pop     r3      /* Restore modified registers */
    ret

/*
 * Output r1 using character display.
 */
decimal_output:
    push    r1          /* Save modified registers */
    push    r2
    mov     r2, r1      /* r2 contains the remaining value to convert */
    mov     r1, 0       /* r1 contains the digit for outputting */

/* TODO: 20s loop, etc */

decimal_100s_loop:
    cmp     r2, 100
    jmp-neg decimal_100s_finish
    sub     r2, 100
    add     r1, 1
    jmp     decimal_100s_loop

decimal_100s_finish:
    st      [0x4005], r1
    mov     r1, 0

decimal_10s_loop:
    cmp     r2, 10
    jmp-neg decimal_10s_finish
    sub     r2, 10
    add     r1, 1
    jmp     decimal_10s_loop

decimal_10s_finish:
    st      [0x4006], r1

decimal_1s:
    st      [0x4007], r2 /* Output the ones */

    /* TODO: There seems to be a bug in call, corrupting registers */
    /* call    show_string */
    pop     r2      /* Restore modified registers */
    pop     r1
    ret

main:
    /* Set the stack pointer into RAM */
    ldi     hl, 0x2000
    mov     sp, hl

    /* Show "SNEPPER1" at 0x4000 */
    /* TODO: Loop / memcpy */
    ld      r1, [0x0010]
    st      [0x4000], r1
    ld      r1, [0x0011]
    st      [0x4001], r1
    ld      r1, [0x0012]
    st      [0x4002], r1
    ld      r1, [0x0013]
    st      [0x4003], r1
    ld      r1, [0x0014]
    st      [0x4004], r1
    ld      r1, [0x0015]
    st      [0x4005], r1
    ld      r1, [0x0016]
    st      [0x4006], r1
    ld      r1, [0x0017]
    st      [0x4007], r1
    call    show_string

    /* Blank display */
    mov     r1, 0x20
    st      [0x4000], r1
    st      [0x4001], r1
    st      [0x4002], r1
    st      [0x4003], r1
    st      [0x4004], r1
    st      [0x4005], r1
    st      [0x4006], r1
    st      [0x4007], r1
    call    show_string

    /* Initialise registers */
    mov     r1, 1
    mov     r2, 0
    mov     r4, 0   /* Loop counter */

loop:
    mov     r3, r2
    mov     r2, r1
    add     r1, r3
    call    decimal_output
    /* Temporarily calling show_string from here until the call bug is fixed */
    call    show_string
    add     r4, 1
    cmp     r4, 11 /* TODO: CMP needs to be improved, as 233 - 100  (133) is detected as negative. Maybe we should check for carry/borrow instead? */
    jmp-z   exit
    jmp     loop

exit:
    halt

