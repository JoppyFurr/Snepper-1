
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
 * Store a single microcode step
 */
void store_step (uint8_t instruction, uint8_t step, uint16_t value )
{
    microcode [instruction << 4 | 0 | step] = value;
    microcode [instruction << 4 | 8 | step] = value; /* For now, ignore the flag bit */
}

/* Note: Does the real ram work immediately? Or does it require a clock to take in the address? */
int instruction = 0;
#define READ_INSTRUCTION store_step(instruction, 0, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_IR  | MISC_PC_COUNT)

/*
 * Generate the microcode
 */
void generate_microcode ()
{
    uint16_t reg_out[4] = { DATA_OUT_R1, DATA_OUT_R2, DATA_OUT_R3, DATA_OUT_R4 };
    uint16_t reg_in[4]  = { DATA_IN_R1,  DATA_IN_R2,  DATA_IN_R3,  DATA_IN_R4  };

    /* mov rX, rX */
    printf ("0x%02x - mov rX, rX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
        for (int src = 0; src < 4; src++)
        {
            READ_INSTRUCTION;
            store_step(instruction, 1, ADDR_OUT_PC | ADDR_IN_none | reg_out[src] | reg_in[dst] | MISC_FINAL_STEP);
            instruction++;
        }
    }
    /* ldi rX, 0xXX */
    printf ("0x%02x - ldi rX, 0xXX\n", instruction);
    for (int dst = 0; dst < 4; dst++)
    {
            READ_INSTRUCTION;
            store_step(instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | reg_in[dst] | MISC_PC_COUNT);
            store_step(instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_none | MISC_FINAL_STEP);  /* Can this be made faster by having a "count-and-final-step" option? */
            instruction++;
    }

    /* mov dh, rX */
    printf ("0x%02x - mov dh, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION;
        store_step(instruction, 1, ADDR_OUT_PC | ADDR_IN_none | reg_out[src] | DATA_IN_DH | MISC_FINAL_STEP);
        instruction++;
    }

    /* mov dl, rX */
    printf ("0x%02x - mov dl, rX\n", instruction);
    for (int src = 0; src < 4; src++)
    {
        READ_INSTRUCTION;
        store_step(instruction, 1, ADDR_OUT_PC | ADDR_IN_none | reg_out[src] | DATA_IN_DL | MISC_FINAL_STEP);
        instruction++;
    }

    /* ldi hl, 0xXXXX */
    printf ("0x%02x - ldi hl, 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH | MISC_PC_COUNT);
    store_step(instruction, 2, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL | MISC_PC_COUNT);
    store_step(instruction, 3, ADDR_OUT_PC | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* mov dc, hl */
    printf ("0x%02x - mov dc, hl\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_DC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* mov sp, hl */
    printf ("0x%02x - mov sp, hl\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_SP | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

#if 0 /* Can be implemented as a macro */
    /* ldi dc, 0xXXXX */
    printf ("0x%02x - ldi dc, 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH | MISC_PC_COUNT);
    store_step(instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL | MISC_PC_COUNT);
    store_step(instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_DC | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;

    /* ldi sp, 0xXXXX */
    printf ("0x%02x - ldi sp, 0xXXXX\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DH | MISC_PC_COUNT);
    store_step(instruction, 2, ADDR_OUT_PC    | ADDR_IN_none | DATA_OUT_MEM | DATA_IN_DL | MISC_PC_COUNT);
    store_step(instruction, 3, ADDR_OUT_DH_DL | ADDR_IN_SP | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;
#endif

    /* unused */
    printf ("0x%02x - unused\n", instruction);
    READ_INSTRUCTION;
    store_step(instruction, 1, ADDR_OUT_DH_DL | ADDR_IN_none | DATA_OUT_R1 | DATA_IN_none | MISC_FINAL_STEP);
    instruction++;


}

/*
 * Output in Digital format
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

    for (int i = 0; i < 256; i++)
    {
        fprintf (output, "%x\n", microcode[i]);
    }

    fclose (output);
}

int main (void)
{
    int rc = EXIT_SUCCESS;

    generate_microcode ();


    /* Write the microcode to a file */
    if (output_digital ())
    {
        rc = EXIT_FAILURE;
    }

    return rc;
}
