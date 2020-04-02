
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum DataDriver_e {
    DATA_OUT_R1     = 0x0000,
    DATA_OUT_R2     = 0x1000,
    DATA_OUT_R3     = 0x2000,
    DATA_OUT_R4     = 0x3000,
    DATA_OUT_MEM    = 0x4000,
    DATA_OUT_AH     = 0x5000,
    DATA_OUT_AL     = 0x6000,
    DATA_OUT_ADD    = 0x7000,
    DATA_OUT_ADD_C  = 0x8000,
    DATA_OUT_SUB    = 0x9000,
    DATA_OUT_SUB_C  = 0xa000,
    DATA_OUT_OR     = 0xb000,
    DATA_OUT_AND    = 0xc000,
    DATA_OUT_XOR    = 0xd000,
    DATA_OUT_CFG    = 0xe000,
    DATA_OUT_INPUT  = 0xf000
} DataDriver;

typedef enum DataConsumer_e {
    DATA_IN_R1      = 0x0000,
    DATA_IN_R2      = 0x0100,
    DATA_IN_R3      = 0x0200,
    DATA_IN_R4      = 0x0300,
    DATA_IN_RAM     = 0x0400,
    DATA_IN_DH      = 0x0500,
    DATA_IN_DL      = 0x0600,
    DATA_IN_A1      = 0x0700,
    DATA_IN_A2      = 0x0800,
    DATA_IN_IR      = 0x0900,
    DATA_IN_none    = 0x0a00,
    DATA_IN_none2   = 0x0b00,
    DATA_IN_none3   = 0x0c00,
    DATA_IN_none4   = 0x0d00,
    DATA_IN_CFG     = 0x0e00,
    DATA_IN_OUTPUT  = 0x0f00
} DataConsumer;

typedef enum AddressDriver_e {
    ADDR_OUT_PC     = 0x0000,
    ADDR_OUT_DC     = 0x0040,
    ADDR_OUT_SP     = 0x0080,
    ADDR_OUT_DH_DL  = 0x00c0,
} AddressDriver;

typedef enum AddressConsumer_e {
    ADDR_IN_PC      = 0x0000,
    ADDR_IN_DC      = 0x0008,
    ADDR_IN_SP      = 0x0010,
    ADDR_IN_AH_AL   = 0x0018,
    ADDR_IN_none    = 0x0020,
    ADDR_IN_none2   = 0x0028,
    ADDR_IN_none3   = 0x0030,
    ADDR_IN_none4   = 0x0038,
} AddressConsumer;

typedef enum Misc_e {
    MISC_PC_COUNT   = 0x0000,
    MISC_DC_COUNT   = 0x0001,
    MISC_SP_INC     = 0x0002,
    MISC_SP_DEC     = 0x0003,
    MISC_none       = 0x0004,
    MISC_none2      = 0x0005,
    MISC_none3      = 0x0006,
    MISC_FINAL_STEP = 0x0007,
} Misc;

uint16_t microcode [8192] = { };

/*
 * Store an unconditional microcode step.
 */
void store_step (uint8_t instruction, uint8_t step, uint16_t value )
{
    microcode [(instruction << 4) | (0 << 3) | step] = value;
    microcode [(instruction << 4) | (1 << 3) | step] = value;
}

/*
 * Store a conditional microcode step.
 */
void store_step_conditional (uint8_t instruction, bool condition, uint8_t step, uint16_t value )
{
    if (condition == false)
    {
        microcode [(instruction << 4) | (0 << 3) | step] = value;
    }
    else
    {
        microcode [(instruction << 4) | (1 << 3) | step] = value;
    }
}

/* Note: Does the real ram work immediately? Or does it require a clock to take in the address? */
int instruction = 0;
#define READ_INSTRUCTION() \
do \
{ \
    store_step (instruction, 0, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_IR  | MISC_PC_COUNT); \
} while (0)

#define ALU_INPUT_STEP_1_2() \
do \
{ \
    store_step (instruction,     1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [dst] | DATA_IN_A1        | MISC_none); \
    if (src == dst) \
    { \
        store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM       | DATA_IN_A2        | MISC_PC_COUNT); \
    } \
    else \
    { \
        store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | DATA_IN_A2        | MISC_none); \
    } \
} while (0)

/* TODO: A number of instructions have an empty final step, just so they can count. Perhaps final-step needs to be in addr-in or data-in? */
#define EMPTY_FINAL_STEP (ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP)

/*
 * Generate the microcode.
 */
void generate_microcode ()
{
    uint16_t data_out_reg [4] = { DATA_OUT_R1, DATA_OUT_R2, DATA_OUT_R3, DATA_OUT_R4 };
    uint16_t data_in_reg  [4] = { DATA_IN_R1,  DATA_IN_R2,  DATA_IN_R3,  DATA_IN_R4  };

    /* mov rX, { rX / 0xXX } */
    printf ("0x%02x - mov rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();

            if (src == dst)
            {
                store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | data_in_reg [dst] | MISC_PC_COUNT);
                store_step (instruction, 2, EMPTY_FINAL_STEP);
            }
            else
            {
                store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | data_in_reg [dst] | MISC_FINAL_STEP);
            }

            instruction++;
        }
    }

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    while (instruction < 0x14)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* mov dh, rX */
    printf ("0x%02x - mov dh, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | DATA_IN_DH | MISC_FINAL_STEP);
        instruction++;
    }

    /* mov dl, rX */
    printf ("0x%02x - mov dl, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | DATA_IN_DL | MISC_FINAL_STEP);
        instruction++;
    }

    /* ldi hl, 0xXXXX */
    printf ("0x%02x - ldi hl, 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step (instruction, 3, EMPTY_FINAL_STEP);
    instruction++;

    /* mov dc, hl */
    printf ("0x%02x - mov dc, hl\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_DC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* mov sp, hl */
    printf ("0x%02x - mov sp, hl\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_SP | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;


    /* ld rX, [dh, rX] */
    printf ("0x%02x - ld rX, [dh, rX]\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | data_out_reg [src] | DATA_IN_DL        | MISC_none);
            store_step (instruction, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_MEM       | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* st [dh, rX], rX */
    printf ("0x%02x - st [dh, rX], rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | data_out_reg [dst] | DATA_IN_DL  | MISC_none);
            store_step (instruction, 2, ADDR_OUT_DH_DL | ADDR_IN_none | data_out_reg [src] | DATA_IN_RAM | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* ld rX, [0xXXXX] */
    printf ("0x%02x - ld rX, [0xXXXX]\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH        | MISC_PC_COUNT);
            store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL        | MISC_PC_COUNT);
            store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_MEM | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
    }

    /* st [0xXXXX], rX */
    printf ("0x%02x - st [0xXXXX], rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM       | DATA_IN_DH  | MISC_PC_COUNT);
            store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM       | DATA_IN_DL  | MISC_PC_COUNT);
            store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_none | data_out_reg [src] | DATA_IN_RAM | MISC_FINAL_STEP);
            instruction++;
    }

    /* ld rX, dc */
    printf ("0x%02x - ld rX, dc\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_DC | ADDR_IN_none | DATA_OUT_MEM | data_in_reg [dst] | MISC_DC_COUNT);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    printf ("0x%02x - st dc, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_DC | ADDR_IN_none | data_out_reg [src] | DATA_IN_RAM | MISC_DC_COUNT);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    /* pop rX */
    printf ("0x%02x - pop rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_SP | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_none      | MISC_SP_DEC);
            store_step (instruction, 2, ADDR_OUT_SP | ADDR_IN_none | DATA_OUT_MEM | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
    }

    /* push rX */
    printf ("0x%02x - push rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION ();
            store_step (instruction, 1, ADDR_OUT_SP | ADDR_IN_none | data_out_reg [src] | DATA_IN_RAM  | MISC_SP_INC);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    while (instruction < 0x60)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* jmp hl */
    printf ("0x%02x - jmp hl\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_PC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* jmp-z hl */
    printf ("0x%02x - jmp-z hl\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true , 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-c hl */
    printf ("0x%02x - jmp-c hl\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-nz hl */
    printf ("0x%02x - jmp-nz hl\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-nc hl */
    printf ("0x%02x - jmp-nc hl\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp 0xXXXX */
    printf ("0x%02x - jmp 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* jmp-z 0xXXXX */
    printf ("0x%02x - jmp-z 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-c 0xXXXX */
    printf ("0x%02x - jmp-c 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-nz 0xXXXX */
    printf ("0x%02x - jmp-nz 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-nc 0xXXXX */
    printf ("0x%02x - jmp-nc 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step_conditional (instruction, false, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* call hl */
    printf ("0x%02x - call hl\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_AH_AL | DATA_OUT_R1 | DATA_IN_none | MISC_none);
    store_step (instruction, 2, ADDR_OUT_SP    | ADDR_IN_none  | DATA_OUT_AH | DATA_IN_RAM  | MISC_SP_INC);
    store_step (instruction, 3, ADDR_OUT_SP    | ADDR_IN_none  | DATA_OUT_AL | DATA_IN_RAM  | MISC_SP_INC);
    store_step (instruction, 4, ADDR_OUT_DH_DL | ADDR_IN_PC    | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* call 0xXXXX */
    printf ("0x%02x - call 0xXXXX\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none  | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none  | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step (instruction, 3, ADDR_OUT_PC    | ADDR_IN_AH_AL | DATA_OUT_R1  | DATA_IN_none | MISC_none);
    store_step (instruction, 4, ADDR_OUT_SP    | ADDR_IN_none  | DATA_OUT_AH  | DATA_IN_RAM  | MISC_SP_INC);
    store_step (instruction, 5, ADDR_OUT_SP    | ADDR_IN_none  | DATA_OUT_AL  | DATA_IN_RAM  | MISC_SP_INC);
    store_step (instruction, 6, ADDR_OUT_DH_DL | ADDR_IN_PC    | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* ret */
    printf ("0x%02x - ret\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_SP_DEC);
    store_step (instruction, 2, ADDR_OUT_SP    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_SP_DEC);
    store_step (instruction, 3, ADDR_OUT_SP    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_none);
    store_step (instruction, 4, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* TODO: Finish I/O Instructions */
    printf ("0x%02x - TODO: input rX\n", instruction);

    while (instruction < 0x74)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* output rX */
    printf ("0x%02x - output rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | DATA_IN_OUTPUT | MISC_FINAL_STEP);
        instruction++;
    }

    printf ("0x%02x - TODO: input dc\n", instruction);
    printf ("0x%02x - TODO: output dc\n", instruction);

    while (instruction < 0x7e)
    {
        READ_INSTRUCTION ();
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* cfg-set */
    printf ("0x%02x - cfg-set\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_A1  | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_CFG | DATA_IN_A2  | MISC_none);
    store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_OR  | DATA_IN_CFG | MISC_FINAL_STEP);
    instruction++;

    /* cfg-clr */
    printf ("0x%02x - cfg-clr\n", instruction);
    READ_INSTRUCTION ();
    store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_A1  | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_CFG | DATA_IN_A2  | MISC_none);
    store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_AND | DATA_IN_CFG | MISC_FINAL_STEP);
    instruction++;

    /* For ALU instructions:
     * -> If src != dst, the source is a register.
     * -> If src == dst, the source is an immediate. */

    /* add rX, { rX / 0xXX } */
    printf ("0x%02x - add rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_ADD | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* addc rX, { rX / 0xXX } */
    printf ("0x%02x - addc rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_ADD_C | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* sub rX, { rX / 0xXX } */
    printf ("0x%02x - sub rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_SUB | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* subc rX, { rX / 0xXX } */
    printf ("0x%02x - subc rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_SUB_C | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* and rX, { rX / 0xXX } */
    printf ("0x%02x - and rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_AND | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* or rX, { rX / 0xXX } */
    printf ("0x%02x - or rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction,     1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [dst] | DATA_IN_A1        | MISC_none);
            if (src == dst)
            {
                store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM       | DATA_IN_A2        | MISC_PC_COUNT);
            }
            else
            {
                store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | data_out_reg [src] | DATA_IN_A2        | MISC_none);
            }
            store_step (instruction,     3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_OR        | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* xor rX, { rX / 0xXX } */
    printf ("0x%02x - xor rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction,     3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_XOR       | data_in_reg [dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* cmp rX, { rX / 0xXX } */
    printf ("0x%02x - cmp rX, { rX / 0xXX }\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION ();
            ALU_INPUT_STEP_1_2 ();
            store_step (instruction,     3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_SUB       | DATA_IN_none      | MISC_FINAL_STEP);
            instruction++;
        }
    }
}

/*
 * Output in Digital format.
 */
int output_digital ()
{
    FILE *output = NULL;

    output = fopen ("microcode.hex", "w");
    if (output == NULL)
    {
        fprintf (stderr, "Unable to open microcode.hex for output.\n");
        return -1;
    }

    fprintf (output, "v2.0 raw\n");

    for (int i = 0; i < (8 << 10); i++)
    {
        fprintf (output, "%x\n", microcode [i]);
    }

    fclose (output);
}

/*
 * Output in format more useful for debugging.
 */
int output_debug ()
{
    FILE *output = NULL;

    output = fopen ("microcode.debug", "w");
    if (output == NULL)
    {
        fprintf (stderr, "Unable to open microcode.debug for output.\n");
        return -1;
    }

    for (int i = 0; i < (8 << 10); i += 16)
    {
        fprintf (output, "%02x f=0: %04x %04x %04x %04x  %04x %04x %04x %04x\n",
                 i >> 4,
                 microcode [i +  0], microcode [i +  1], microcode [i +  2], microcode [i +  3],
                 microcode [i +  4], microcode [i +  5], microcode [i +  6], microcode [i +  7]);
        fprintf (output, "%02x f=1: %04x %04x %04x %04x  %04x %04x %04x %04x\n",
                 i >> 4,
                 microcode [i +  8], microcode [i +  9], microcode [i + 10], microcode [i + 11],
                 microcode [i + 12], microcode [i + 13], microcode [i + 14], microcode [i + 15]);
    }

    fclose (output);
}

int main (void)
{
    int rc = EXIT_SUCCESS;

    generate_microcode ();


    /* Write the microcode to a file for the simulator */
    if (output_digital ())
    {
        rc = EXIT_FAILURE;
    }

    /* Write the microcode to a file for investigation */
    if (output_debug ())
    {
        rc = EXIT_FAILURE;
    }

    return rc;
}
