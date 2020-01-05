
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum Instruction_e {

    MOV_R1_R1       = 0x00,
    MOV_R1_R2       = 0x01,
    MOV_R1_R3       = 0x02,
    MOV_R1_R4       = 0x03,
    MOV_R2_R1       = 0x04,
    MOV_R2_R2       = 0x05,
    MOV_R2_R3       = 0x06,
    MOV_R2_R4       = 0x07,
    MOV_R3_R1       = 0x08,
    MOV_R3_R2       = 0x09,
    MOV_R3_R3       = 0x0a,
    MOV_R3_R4       = 0x0b,
    MOV_R4_R1       = 0x0c,
    MOV_R4_R2       = 0x0d,
    MOV_R4_R3       = 0x0e,
    MOV_R4_R4       = 0x0f,

    LDI_R1_XX       = 0x10,
    LDI_R2_XX       = 0x11,
    LDI_R3_XX       = 0x12,
    LDI_R4_XX       = 0x13,

    MOV_DH_R1       = 0x14,
    MOV_DH_R2       = 0x15,
    MOV_DH_R3       = 0x16,
    MOV_DH_R4       = 0x17,
    MOV_DL_R1       = 0x18,
    MOV_DL_R2       = 0x19,
    MOV_DL_R3       = 0x1a,
    MOV_DL_R4       = 0x1b,

    MOV_HL_XXXX     = 0x1c,
    MOV_DC_HL       = 0x1d,
    MOV_SP_HL       = 0x1e,
    UNUSED_1F       = 0x1f,

} Instruction;

#define NOP MOV_R1_R1

/*
 * Test program.
 */
uint8_t test_program[] =
{
    LDI_R1_XX, 0xde,
    LDI_R2_XX, 0xad,
    LDI_R3_XX, 0xbe,
    LDI_R4_XX, 0xef,
    NOP,
    NOP,
    NOP,
};

/*
 * Output in Digital format.
 */
int output_digital ()
{
    FILE *output = NULL;

    output = fopen ("rom.hex", "w");
    if (output == NULL)
    {
        fprintf (stderr, "Unable to open rom.hex for output.\n");
        return -1;
    }

    fprintf (output, "v2.0 raw\n");

    for (int i = 0; i < sizeof (test_program); i++)
    {
        fprintf (output, "%x\n", test_program[i]);
    }

    fclose (output);
}

int main (void)
{
    int rc = EXIT_SUCCESS;

    /* Write the rom to a file for the simulator */
    if (output_digital ())
    {
        rc = EXIT_FAILURE;
    }

    return rc;
}
