/*
 * Test program for the Snepper-1 assembler.
 * Counts in a loop, using a function to output.
 *
 * Memory map:
 *
 *  0x1000 - Code
 *  0x2000 - Stack
 *  0x3000 - fib n
 *  0x3002 - fib n-1
 *  0x4000 - Display
 *
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
 * Blank the display
 */
blank_display:
    push    r1
    mov     r1, 0x20
    cfg-clr 0xf0
    cfg-set 0x00
    output  r1
    cfg-clr 0xf0
    cfg-set 0x01
    output  r1
    cfg-clr 0xf0
    cfg-set 0x02
    output  r1
    cfg-clr 0xf0
    cfg-set 0x03
    output  r1
    cfg-clr 0xf0
    cfg-set 0x04
    output  r1
    cfg-clr 0xf0
    cfg-set 0x05
    output  r1
    cfg-clr 0xf0
    cfg-set 0x06
    output  r1
    cfg-clr 0xf0
    cfg-set 0x07
    output  r1
    pop     r1
    ret

/*
 * Show "SNEPPER1"
 */
logo:
    push    r1
    mov     hl, 0x4000
    mov     dc, hl
    ld      r1, [0x0010]
    st      [dc++], r1
    ld      r1, [0x0011]
    st      [dc++], r1
    ld      r1, [0x0012]
    st      [dc++], r1
    ld      r1, [0x0013]
    st      [dc++], r1
    ld      r1, [0x0014]
    st      [dc++], r1
    ld      r1, [0x0015]
    st      [dc++], r1
    ld      r1, [0x0016]
    st      [dc++], r1
    ld      r1, [0x0017]
    st      [dc++], r1
    call    show_string
    pop     r1
    ret

/*
 * Replace leading zeros with whitespace
 */
whitespace_pad:
    push    r1
    push    r2
    push    r3
    push    r4

    mov     r2, 0x20    /* whitespace */
    mov     r3, 0       /* Index */
    mov     r4, 0x40    /* High byte of address */

whitespace_loop:

    mov     rh, r4
    ld      r1, [rh, r3]
    cmp     r1, 0
    jmp-nz  whitespace_done
    st      [rh, r3], r2

    add     r3, 1
    cmp     r3, 7
    jmp-nz  whitespace_loop

whitespace_done:
    pop     r4
    pop     r3
    pop     r2
    pop     r1
    ret

/*
 * Display the 16-bit number stored at 0x3000
 */
decimal_output:
    push    r1          /* Save modified registers */
    push    r2
    push    r4

    /*
     * r1 - Remaining high
     * r2 - Remaining low
     * r3 - temp
     * r4 - Digit to output
     */

    ld      r1, [0x3000]
    ld      r2, [0x3001]

    /* Zero the unused digits */
    mov     r4, 0
    st      [0x4000], r4
    st      [0x4001], r4
    st      [0x4002], r4

decimal_10_000s_loop:
    sub     r2, 0x10 /* 10000_low  */
    sub-c   r1, 0x27 /* 10000_high */
    jmp-c   decimal_10_000s_finish
    add     r4, 1
    jmp     decimal_10_000s_loop
decimal_10_000s_finish:
    /* Add the last 10000 back */
    add     r2, 0x10 /* 10000_low  */
    add-c   r1, 0x27 /* 10000_high */
    st      [0x4003], r4
    mov     r4, 0

decimal_1_000s_loop:
    sub     r2, 0xe8 /* 1000_low  */
    sub-c   r1, 0x03 /* 1000_high */
    jmp-c   decimal_1_000s_finish
    add     r4, 1
    jmp     decimal_1_000s_loop
decimal_1_000s_finish:
    /* Add the last 1000 back */
    add     r2, 0xe8 /* 1000_low  */
    add-c   r1, 0x03 /* 1000_high */
    st      [0x4004], r4
    mov     r4, 0

decimal_100s_loop:
    sub     r2, 0x64 /* 100_low  */
    sub-c   r1, 0x00 /* 100_high */
    jmp-c   decimal_100s_finish
    add     r4, 1
    jmp     decimal_100s_loop
decimal_100s_finish:
    /* Add the last 100 back */
    add     r2, 0x64 /* 100_low  */
    add-c   r1, 0x00 /* 100_high */
    st      [0x4005], r4
    mov     r4, 0

decimal_10s_loop:
    cmp     r2, 10
    jmp-c   decimal_10s_finish
    sub     r2, 10
    add     r4, 1
    jmp     decimal_10s_loop
decimal_10s_finish:
    st      [0x4006], r4

decimal_1s:
    st      [0x4007], r2 /* Output the ones */

    call    whitespace_pad
    call    blank_display
    call    show_string

    pop     r4      /* Restore modified registers */
    pop     r2
    pop     r1
    ret

/*
 * Entry point
 */
main:
    /* Set the stack pointer into RAM */
    mov     hl, 0x2000
    mov     sp, hl

    call    logo

    /*
     * Initialise memory
     * 0x3000 - fib(n)
     * 0x3002 - fib(n-1)
     * 0x3004 - fib(n-2)
     * r4 - loop count
     */
    mov     r1, 0
    st      [0x3000], r1
    st      [0x3001], r1
    st      [0x3002], r1
    st      [0x3003], r1
    st      [0x3004], r1
    mov     r1, 1
    st      [0x3005], r1
    mov     r4, 0

main_loop:
    /* TODO: fib(n) = fib(n-1) + fib(n-2) */
    ld      r1, [0x3003]
    ld      r2, [0x3005]
    add     r1, r2
    st      [0x3001], r1
    ld      r1, [0x3002]
    ld      r2, [0x3004]
    add-c   r1, r2
    st      [0x3000], r1

    call    decimal_output

    /* fib(n-2) = fib(n-1) */
    ld      r1, [0x3002]
    st      [0x3004], r1
    ld      r1, [0x3003]
    st      [0x3005], r1

    /* fib(n-1) = fib(n) */
    ld      r1, [0x3000]
    st      [0x3002], r1
    ld      r1, [0x3001]
    st      [0x3003], r1

    /* Loop for 20 Fibonacci numbers */
    add     r4, 1
    cmp     r4, 24
    jmp-z   exit

    jmp     main_loop

exit:
    halt

