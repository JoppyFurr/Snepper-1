
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum Instruction_e {

    /* Move / Load Immediate */
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

    LDI_HL_XXXX     = 0x1c,
    MOV_DC_HL       = 0x1d,
    MOV_SP_HL       = 0x1e,
    UNUSED_1F       = 0x1f,

    /* Memory Instructions */
    LD_R1_DH_R1     = 0x20,
    LD_R1_DH_R2     = 0x21,
    LD_R1_DH_R3     = 0x22,
    LD_R1_DH_R4     = 0x23,
    LD_R2_DH_R1     = 0x24,
    LD_R2_DH_R2     = 0x25,
    LD_R2_DH_R3     = 0x26,
    LD_R2_DH_R4     = 0x27,
    LD_R3_DH_R1     = 0x28,
    LD_R3_DH_R2     = 0x29,
    LD_R3_DH_R3     = 0x2a,
    LD_R3_DH_R4     = 0x2b,
    LD_R4_DH_R1     = 0x2c,
    LD_R4_DH_R2     = 0x2d,
    LD_R4_DH_R3     = 0x2e,
    LD_R4_DH_R4     = 0x2f,

    ST_DH_R1_R1     = 0x30,
    ST_DH_R1_R2     = 0x31,
    ST_DH_R1_R3     = 0x32,
    ST_DH_R1_R4     = 0x33,
    ST_DH_R2_R1     = 0x34,
    ST_DH_R2_R2     = 0x35,
    ST_DH_R2_R3     = 0x36,
    ST_DH_R2_R4     = 0x37,
    ST_DH_R3_R1     = 0x38,
    ST_DH_R3_R2     = 0x39,
    ST_DH_R3_R3     = 0x3a,
    ST_DH_R3_R4     = 0x3b,
    ST_DH_R4_R1     = 0x3c,
    ST_DH_R4_R2     = 0x3d,
    ST_DH_R4_R3     = 0x3e,
    ST_DH_R4_R4     = 0x3f,

    LD_R1_XXXX      = 0x40,
    LD_R2_XXXX      = 0x41,
    LD_R3_XXXX      = 0x42,
    LD_R4_XXXX      = 0x43,

    ST_XXXX_R1      = 0x44,
    ST_XXXX_R2      = 0x45,
    ST_XXXX_R3      = 0x46,
    ST_XXXX_R4      = 0x47,

    LD_R1_DC        = 0x48,
    LD_R2_DC        = 0x49,
    LD_R3_DC        = 0x4a,
    LD_R4_DC        = 0x4b,

    ST_DC_R1        = 0x4c,
    ST_DC_R2        = 0x4d,
    ST_DC_R3        = 0x4e,
    ST_DC_R4        = 0x4f,

    POP_R1          = 0x50,
    POP_R2          = 0x51,
    POP_R3          = 0x52,
    POP_R4          = 0x53,

    PUSH_R1         = 0x54,
    PUSH_R2         = 0x55,
    PUSH_R3         = 0x56,
    PUSH_R4         = 0x57,

    UNUSED_58       = 0x58,
    UNUSED_59       = 0x59,
    UNUSED_5A       = 0x5a,
    UNUSED_5B       = 0x5b,
    UNUSED_5C       = 0x5c,
    UNUSED_5D       = 0x5d,
    UNUSED_5E       = 0x5e,
    UNUSED_5F       = 0x5f,

    /* Jump Instructions */
    JMP_HL          = 0x60,
    /* TODO */

    /* TEMPORARY OUTPUT REGISTER */
    TEMP_OUTPUT_R1       = 0x70,

} Instruction;

#define NOP MOV_R1_R1

/*
 * Test program.
 */
uint8_t test_program[] =
{
    /* 0x0000 */ LDI_R1_XX, 0xde,
    /* 0x0002 */ TEMP_OUTPUT_R1,
    /* 0x0003 */ LDI_R1_XX, 0xad,
    /* 0x0005 */ TEMP_OUTPUT_R1,
    /* 0x0006 */ LDI_R1_XX, 0xbe,
    /* 0x0008 */ TEMP_OUTPUT_R1,
    /* 0x0009 */ LDI_R1_XX, 0xef,
    /* 0x000b */ TEMP_OUTPUT_R1,
    /* 0x000c */ LDI_HL_XXXX, 0x00, 0x00,
    /* 0x000f */ JMP_HL,
    /* 0x0010 */ NOP,
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
