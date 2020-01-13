
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
    DATA_OUT_RSHIFT = 0xe000,
    DATA_OUT_IO     = 0xf000
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
    DATA_IN_none5   = 0x0e00,
    DATA_IN_IO      = 0x0f00
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

uint16_t microcode[8192] = { };

/*
 * Store an unconditional microcode step.
 */
void store_step (uint8_t instruction, uint8_t step, uint16_t value )
{
    microcode [ (instruction << 4) | (0 << 3) | step] = value;
    microcode [ (instruction << 4) | (1 << 3) | step] = value;
}

/*
 * Store a conditional microcode step.
 */
void store_step_conditional (uint8_t instruction, bool condition, uint8_t step, uint16_t value )
{
    if (condition == false)
    {
        microcode [ (instruction << 4) | (0 << 3) | step] = value;
    }
    else
    {
        microcode [ (instruction << 4) | (1 << 3) | step] = value;
    }
}

/* Note: Does the real ram work immediately? Or does it require a clock to take in the address? */
int instruction = 0;
#define READ_INSTRUCTION store_step (instruction, 0, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_IR  | MISC_PC_COUNT)

/* TODO: A number of instructions have an empty final step, just so they can count. Perhaps final-step needs to be in addr-in or data-in? */
#define EMPTY_FINAL_STEP (ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP)

/*
 * Generate the microcode.
 */
void generate_microcode ()
{
    uint16_t data_out_reg[4] = { DATA_OUT_R1, DATA_OUT_R2, DATA_OUT_R3, DATA_OUT_R4 };
    uint16_t data_in_reg[4]  = { DATA_IN_R1,  DATA_IN_R2,  DATA_IN_R3,  DATA_IN_R4  };

    /* mov rX, rX */
    printf ("0x%02x - mov rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg[src] | data_in_reg[dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* ldi rX, 0xXX */
    printf ("0x%02x - ldi rX, 0xXX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | data_in_reg[dst] | MISC_PC_COUNT);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    /* mov dh, rX */
    printf ("0x%02x - mov dh, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION;
        store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg[src] | DATA_IN_DH | MISC_FINAL_STEP);
        instruction++;
    }

    /* mov dl, rX */
    printf ("0x%02x - mov dl, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION;
        store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | data_out_reg[src] | DATA_IN_DL | MISC_FINAL_STEP);
        instruction++;
    }

    /* ldi hl, 0xXXXX */
    printf ("0x%02x - ldi hl, 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step (instruction, 3, EMPTY_FINAL_STEP);
    instruction++;

    /* mov dc, hl */
    printf ("0x%02x - mov dc, hl\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_DC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* mov sp, hl */
    printf ("0x%02x - mov sp, hl\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_SP | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;


    /* ld rX, [dh, rX] */
    printf ("0x%02x - ld rX, [dh, rX]\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | data_out_reg[src] | DATA_IN_DL       | MISC_none);
            store_step (instruction, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_MEM      | data_in_reg[dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* st [dh, rX], rX */
    printf ("0x%02x - st [dh, rX], rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | data_out_reg[dst] | DATA_IN_DL  | MISC_none);
            store_step (instruction, 2, ADDR_OUT_DH_DL | ADDR_IN_none | data_out_reg[src] | DATA_IN_RAM | MISC_FINAL_STEP);
            instruction++;
        }
    }

    /* ld rX, [0xXXXX] */
    printf ("0x%02x - ld rX, [0xXXXX]\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH       | MISC_PC_COUNT);
            store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL       | MISC_PC_COUNT);
            store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_MEM | data_in_reg[dst] | MISC_FINAL_STEP);
            instruction++;
    }

    /* st [0xXXXX], rX */
    printf ("0x%02x - st [0xXXXX], rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM      | DATA_IN_DH  | MISC_PC_COUNT);
            store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM      | DATA_IN_DL  | MISC_PC_COUNT);
            store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_none | data_out_reg[src] | DATA_IN_RAM | MISC_FINAL_STEP);
            instruction++;
    }

    /* ld rX, dc */
    printf ("0x%02x - ld rX, dc\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_DC | ADDR_IN_none | DATA_OUT_MEM      | data_in_reg[dst] | MISC_DC_COUNT);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    printf ("0x%02x - st dc, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_DC | ADDR_IN_none | data_out_reg[src] | DATA_IN_RAM | MISC_DC_COUNT);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    /* pop rX */
    printf ("0x%02x - pop rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_SP | ADDR_IN_none | DATA_OUT_MEM | data_in_reg[dst] | MISC_SP_DEC);
            store_step (instruction, 2, EMPTY_FINAL_STEP);
            instruction++;
    }

    /* push rX */
    printf ("0x%02x - push rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
            READ_INSTRUCTION;
            store_step (instruction, 1, ADDR_OUT_SP | ADDR_IN_none | DATA_OUT_MEM      | DATA_IN_none | MISC_SP_INC);
            store_step (instruction, 2, ADDR_OUT_SP | ADDR_IN_none | data_out_reg[src] | DATA_IN_RAM  | MISC_FINAL_STEP);
            instruction++;
    }

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    while (instruction < 0x60)
    {
        READ_INSTRUCTION;
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* jmp hl */
    printf ("0x%02x - jmp hl\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_PC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* jmp-z hl */
    printf ("0x%02x - jmp-z hl\n", instruction);
    READ_INSTRUCTION;
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true , 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-neg hl */
    printf ("0x%02x - jmp-neg hl\n", instruction);
    READ_INSTRUCTION;
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-nz hl */
    printf ("0x%02x - jmp-nz hl\n", instruction);
    READ_INSTRUCTION;
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp-pos hl */
    printf ("0x%02x - jmp-pos hl\n", instruction);
    READ_INSTRUCTION;
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* jmp 0xXXXX */
    printf ("0x%02x - jmp 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step (instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step (instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* jmp-z 0xXXXX */
    printf ("0x%02x - jmp-z 0xXXXX\n", instruction);
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    READ_INSTRUCTION;
    instruction++;

    /* jmp-neg 0xXXXX */
    printf ("0x%02x - jmp-neg 0xXXXX\n", instruction);
    store_step_conditional (instruction, false, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    READ_INSTRUCTION;
    instruction++;

    /* jmp-nz 0xXXXX */
    printf ("0x%02x - jmp-nz 0xXXXX\n", instruction);
    store_step_conditional (instruction, false, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    READ_INSTRUCTION;
    instruction++;

    /* jmp-pos 0xXXXX */
    printf ("0x%02x - jmp-pos 0xXXXX\n", instruction);
    store_step_conditional (instruction, false, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL   | MISC_PC_COUNT);
    store_step_conditional (instruction, false, 3, ADDR_OUT_DH_DL | ADDR_IN_PC   | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    store_step_conditional (instruction, true,  1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  2, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_PC_COUNT);
    store_step_conditional (instruction, true,  3, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1  | DATA_IN_none | MISC_FINAL_STEP);
    READ_INSTRUCTION;
    instruction++;

    /* TODO: call hl */
    printf ("0x%02x - TODO: call hl\n", instruction);
    READ_INSTRUCTION;
    instruction++;

    /* TODO: call 0xXXXX */
    printf ("0x%02x - TODO: call 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    instruction++;

    /* TODO: ret */
    printf ("0x%02x - TODO: ret\n", instruction);
    READ_INSTRUCTION;
    instruction++;

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION;
    store_step (instruction, 1, EMPTY_FINAL_STEP);
    instruction++;

    /* TODO: I/O Instructions */
    printf ("0x%02x - TODO: I/O Instructions\n", instruction);

    READ_INSTRUCTION;
    store_step (instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_IO | MISC_FINAL_STEP);
    instruction++;

    while (instruction < 0x80)
    {
        READ_INSTRUCTION;
        store_step (instruction, 1, EMPTY_FINAL_STEP);
        instruction++;
    }

    /* TODO: add rX, rX */
    printf ("0x%02x - TODO: add rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }

    /* TODO: addc rX, rX */
    printf ("0x%02x - TODO: addc rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }

    /* TODO: sub rX, rX */
    printf ("0x%02x - TODO: sub rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: subc rX, rX */
    printf ("0x%02x - TODO: subc rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: and rX, rX */
    printf ("0x%02x - TODO: and rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: or rX, rX */
    printf ("0x%02x - TODO: or rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: xor rX, rX */
    printf ("0x%02x - TODO: xor rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: lshift rX, {1,2} */
    printf ("0x%02x - TODO: lshift rX, {1,2}\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int shift = 0; shift < 2; shift++)
        {
            READ_INSTRUCTION;
            instruction++;
        }
    }
    /* TODO: rshift rX, {1,2} */
    printf ("0x%02x - TODO: rshift rX, {1,2}\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int shift = 0; shift < 2; shift++)
        {
            READ_INSTRUCTION;
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
        fprintf (output, "%x\n", microcode[i]);
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
                 microcode[i +  0], microcode[i +  1], microcode[i +  2], microcode[i +  3],
                 microcode[i +  4], microcode[i +  5], microcode[i +  6], microcode[i +  7]);
        fprintf (output, "%02x f=1: %04x %04x %04x %04x  %04x %04x %04x %04x\n",
                 i >> 4,
                 microcode[i +  8], microcode[i +  9], microcode[i + 10], microcode[i + 11],
                 microcode[i + 12], microcode[i + 13], microcode[i + 14], microcode[i + 15]);
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
