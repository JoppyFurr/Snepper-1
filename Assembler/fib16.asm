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
 * Output the 16-bit number stored at 0x3000 as decimal
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
    mov     r4, 0

decimal_1000s_loop:
    sub     r2, 0xe8 /* 1000_low  */
    sub-c   r1, 0x03 /* 1000_high */
    jmp-c   decimal_1000s_finish
    add     r4, 1
    jmp     decimal_1000s_loop
decimal_1000s_finish:
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

    /* Show "SNEPPER1" at 0x4000 */
    /* TODO: Loop / memcpy */
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

    /* Blank display */
    mov     r1, 0x20
    mov     hl, 0x4000
    mov     dc, hl
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    st      [dc++], r1
    call    show_string

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

loop:
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
    cmp     r4, 20
    jmp-z   exit

    jmp     loop

exit:
    halt

