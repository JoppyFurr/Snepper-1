/*
 * Assembler for the Snepper-1.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    UNUSED_61       = 0x61,
    JMP_Z_HL        = 0x62,
    JMP_NEG_HL      = 0x63,
    JMP_NZ_HL       = 0x64,
    JMP_POS_HL      = 0x65,
    JMP_XXXX        = 0x66,
    UNUSED_67       = 0x67,
    JMP_Z_XXXX      = 0x68,
    JMP_NEG_XXXX    = 0x69,
    JMP_NZ_XXXX     = 0x6a,
    JMP_POS_XXXX    = 0x6b,
    CALL_HL         = 0x6c,
    CALL_XXXX       = 0x6d,
    RET             = 0x6e,
    UNUSED_6f       = 0x6f,

    /* TODO: I/O Block */
    TEMP_OUTPUT_R1  = 0x70, /* TEMPORARY OUTPUT REGISTER */

    /* ALU Commands */
    ADD_R1_R1       = 0x80,
    ADD_R1_R2       = 0x81,
    ADD_R1_R3       = 0x82,
    ADD_R1_R4       = 0x83,
    ADD_R2_R1       = 0x84,
    ADD_R2_R2       = 0x85,
    ADD_R2_R3       = 0x86,
    ADD_R2_R4       = 0x87,
    ADD_R3_R1       = 0x88,
    ADD_R3_R2       = 0x89,
    ADD_R3_R3       = 0x8a,
    ADD_R3_R4       = 0x8b,
    ADD_R4_R1       = 0x8c,
    ADD_R4_R2       = 0x8d,
    ADD_R4_R3       = 0x8e,
    ADD_R4_R4       = 0x8f,

    ADDC_R1_R1      = 0x90,
    ADDC_R1_R2      = 0x91,
    ADDC_R1_R3      = 0x92,
    ADDC_R1_R4      = 0x93,
    ADDC_R2_R1      = 0x94,
    ADDC_R2_R2      = 0x95,
    ADDC_R2_R3      = 0x96,
    ADDC_R2_R4      = 0x97,
    ADDC_R3_R1      = 0x98,
    ADDC_R3_R2      = 0x99,
    ADDC_R3_R3      = 0x9a,
    ADDC_R3_R4      = 0x9b,
    ADDC_R4_R1      = 0x9c,
    ADDC_R4_R2      = 0x9d,
    ADDC_R4_R3      = 0x9e,
    ADDC_R4_R4      = 0x9f,

    SUB_R1_R1       = 0xa0,
    SUB_R1_R2       = 0xa1,
    SUB_R1_R3       = 0xa2,
    SUB_R1_R4       = 0xa3,
    SUB_R2_R1       = 0xa4,
    SUB_R2_R2       = 0xa5,
    SUB_R2_R3       = 0xa6,
    SUB_R2_R4       = 0xa7,
    SUB_R3_R1       = 0xa8,
    SUB_R3_R2       = 0xa9,
    SUB_R3_R3       = 0xaa,
    SUB_R3_R4       = 0xab,
    SUB_R4_R1       = 0xac,
    SUB_R4_R2       = 0xad,
    SUB_R4_R3       = 0xae,
    SUB_R4_R4       = 0xaf,

    SUBC_R1_R1      = 0xb0,
    SUBC_R1_R2      = 0xb1,
    SUBC_R1_R3      = 0xb2,
    SUBC_R1_R4      = 0xb3,
    SUBC_R2_R1      = 0xb4,
    SUBC_R2_R2      = 0xb5,
    SUBC_R2_R3      = 0xb6,
    SUBC_R2_R4      = 0xb7,
    SUBC_R3_R1      = 0xb8,
    SUBC_R3_R2      = 0xb9,
    SUBC_R3_R3      = 0xba,
    SUBC_R3_R4      = 0xbb,
    SUBC_R4_R1      = 0xbc,
    SUBC_R4_R2      = 0xbd,
    SUBC_R4_R3      = 0xbe,
    SUBC_R4_R4      = 0xbf,

    AND_R1_R1       = 0xc0,
    AND_R1_R2       = 0xc1,
    AND_R1_R3       = 0xc2,
    AND_R1_R4       = 0xc3,
    AND_R2_R1       = 0xc4,
    AND_R2_R2       = 0xc5,
    AND_R2_R3       = 0xc6,
    AND_R2_R4       = 0xc7,
    AND_R3_R1       = 0xc8,
    AND_R3_R2       = 0xc9,
    AND_R3_R3       = 0xca,
    AND_R3_R4       = 0xcb,
    AND_R4_R1       = 0xcc,
    AND_R4_R2       = 0xcd,
    AND_R4_R3       = 0xce,
    AND_R4_R4       = 0xcf,

    OR_R1_R1       = 0xd0,
    OR_R1_R2       = 0xd1,
    OR_R1_R3       = 0xd2,
    OR_R1_R4       = 0xd3,
    OR_R2_R1       = 0xd4,
    OR_R2_R2       = 0xd5,
    OR_R2_R3       = 0xd6,
    OR_R2_R4       = 0xd7,
    OR_R3_R1       = 0xd8,
    OR_R3_R2       = 0xd9,
    OR_R3_R3       = 0xda,
    OR_R3_R4       = 0xdb,
    OR_R4_R1       = 0xdc,
    OR_R4_R2       = 0xdd,
    OR_R4_R3       = 0xde,
    OR_R4_R4       = 0xdf,

    XOR_R1_R1       = 0xe0,
    XOR_R1_R2       = 0xe1,
    XOR_R1_R3       = 0xe2,
    XOR_R1_R4       = 0xe3,
    XOR_R2_R1       = 0xe4,
    XOR_R2_R2       = 0xe5,
    XOR_R2_R3       = 0xe6,
    XOR_R2_R4       = 0xe7,
    XOR_R3_R1       = 0xe8,
    XOR_R3_R2       = 0xe9,
    XOR_R3_R3       = 0xea,
    XOR_R3_R4       = 0xeb,
    XOR_R4_R1       = 0xec,
    XOR_R4_R2       = 0xed,
    XOR_R4_R3       = 0xee,
    XOR_R4_R4       = 0xef,

    LSHIFT_R1_1     = 0xf0,
    LSHIFT_R1_2     = 0xf1,
    LSHIFT_R2_1     = 0xf2,
    LSHIFT_R2_2     = 0xf3,
    LSHIFT_R3_1     = 0xf4,
    LSHIFT_R3_2     = 0xf5,
    LSHIFT_R4_1     = 0xf6,
    LSHIFT_R4_2     = 0xf7,

    RSHIFT_R1_1     = 0xf8,
    RSHIFT_R1_2     = 0xf9,
    RSHIFT_R2_1     = 0xfa,
    RSHIFT_R2_2     = 0xfb,
    RSHIFT_R3_1     = 0xfc,
    RSHIFT_R3_2     = 0xfd,
    RSHIFT_R4_1     = 0xfe,
    RSHIFT_R4_2     = 0xff,
} Instruction;

#define NOP MOV_R1_R1

uint8_t rom [8192] = { 0 };

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

    for (int i = 0; i < sizeof (rom); i++)
    {
        fprintf (output, "%x\n", rom [i]);
    }

    fclose (output);
}

#define SCAN_NEXT_TOKEN() \
do \
{ \
    if (fscanf (source, "%s", buffer) == EOF) \
    { \
        fprintf (stderr, "Error: Unexpected end of input.\n"); \
        return -1; \
    } \
} while (0)

#define PARSE_INT(value, ptr) \
do \
{ \
    char *endptr = NULL; \
    value = strtoul (ptr, & endptr, 0); \
    if (endptr == ptr) \
    { \
        fprintf (stderr, "Error: Unexpected input \"%s\". Expected integer.\n", ptr); \
        return -1; \
    } \
} while (0)

#define PARSE_HEX_INT(value, ptr) \
do \
{ \
    char *endptr = NULL; \
    value = strtoul (ptr, & endptr, 16); \
    if (endptr == ptr) \
    { \
        fprintf (stderr, "Error: Unexpected input \"%s\". Expected hex integer.\n", ptr); \
        return -1; \
    } \
} while (0)

#define PARSE_REG(reg, ptr) \
do \
{ \
    ptr [2] = '\0'; \
    reg = strtoul (& ptr [1], NULL, 16) - 1; \
    if (reg >= 4) \
    { \
        fprintf (stderr, "Error: Invalid register \"%s\".\n", ptr); \
        return -1; \
    } \
} while (0)

#define CONFIRM_MATCH(expected, input) \
do \
{ \
    if (strcmp (expected, input) != 0) \
    { \
        fprintf (stderr, "Error: Unexpected input \"%s\". Expected \"%s\"\n", input, expected); \
        return -1; \
    } \
} while (0)



int parse_asm (FILE *source)
{
    uint16_t address = 0;

    char buffer [80] = { '\0' };

    uint8_t dst;
    uint8_t src;

    while (fscanf (source, "%s", buffer) != EOF)
    {
        /* Comment */
        if (strcmp ("/*", buffer) == 0)
        {
            do
            {
                SCAN_NEXT_TOKEN ();
            } while (strcmp ("*/", buffer) != 0);

            continue;
        }

        /* Offset */
        if (strncmp (".", buffer, 1) == 0)
        {
            PARSE_HEX_INT (address, & buffer [1]);

            continue;
        }

        /* mov */
        if (strcmp ("mov", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* mov rX, rX */
            if (strncmp ("r", buffer, 1) == 0)
            {
                PARSE_REG (dst, buffer);
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = MOV_R1_R1 + (dst << 2) + (src << 0);
            }

            /* mov dh, rX */
            else if (strncmp ("dh", buffer, 2) == 0)
            {
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = MOV_DH_R1 + (src << 0);
            }

            /* mov dl, rX */
            else if (strncmp ("dl", buffer, 2) == 0)
            {
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = MOV_DL_R1 + (src << 0);
            }

            /* mov dc, hl */
            else if (strncmp ("dc", buffer, 2) == 0)
            {
                SCAN_NEXT_TOKEN ();
                CONFIRM_MATCH ("hl", buffer);
                rom [address++] = MOV_DC_HL;
            }

            /* mov sp, hl */
            else if (strncmp ("sp", buffer, 2) == 0)
            {
                SCAN_NEXT_TOKEN ();
                CONFIRM_MATCH ("hl", buffer);
                rom [address++] = MOV_SP_HL;
            }

            continue;
        }

        /* ldi */
        /* ld */
        /* st */
        /* pop */
        /* push */
        /* jmp */
        /* call */
        /* ret */

        fprintf (stderr, "Warning: Unexpected input \"%s\".\n", buffer);
    }
}

int main (int argc, char **argv)
{
    int rc = EXIT_SUCCESS;
    FILE *source = NULL;

    if (argc != 2)
    {
        fprintf (stderr, "Usage: %s <source.asm>\n", argv [0]);
        return EXIT_FAILURE;
    }

    source = fopen (argv [1], "r");

    if (source == NULL)
    {
        fprintf (stderr, "Error: Unable to open %s.\n", argv [1]);
        return EXIT_FAILURE;
    }

    if (parse_asm (source) == -1)
    {
        return EXIT_FAILURE;
    }

    /* Write the rom to a file for the simulator */
    if (output_digital ())
    {
        rc = EXIT_FAILURE;
    }

    return rc;
}
