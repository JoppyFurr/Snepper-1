/*
 * Assembler for the Snepper-1.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "instructions.h"

typedef struct Label_s
{
    char *name;
    uint16_t address;
    uint16_t *references;
    uint16_t reference_count;
} Label;

Label **labels = NULL;
uint16_t label_count = 0;
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

bool label_exists (char *name)
{
    for (int i = 0; i < label_count; i++)
    {
        if (strcmp (labels [i]->name, name) == 0)
        {
            return true;
        }
    }

    return false;
}

int label_add (char *name)
{
    if (label_exists (name))
    {
        fprintf (stderr, "Error: Label \"%s\" already exists.\n", name);
        return -1;
    }

    labels = realloc (labels, (label_count + 1) * sizeof (Label *));
    labels [label_count] = malloc (sizeof (Label));
    labels [label_count]->name = strdup (name);
    labels [label_count]->address = 0x0000;
    labels [label_count]->references = NULL;
    labels [label_count]->reference_count = 0;
    label_count++;

    return 0;
}

int label_set_address (char *name, uint16_t address)
{
    for (int i = 0; i < label_count; i++)
    {
        if (strcmp (labels [i]->name, name) == 0)
        {
            labels [i]->address = address;
            return 0;
        }
    }

    return -1;
}

#define SCAN_NEXT_TOKEN() \
do \
{ \
    if (fscanf (source, "%79s", buffer) == EOF) \
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
    if (ptr[0] != 'r') \
    { \
        fprintf (stderr, "Error: Invalid register \"%s\".\n", ptr); \
        return -1; \
    } \
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

#define UNEXPECTED(input) \
do \
{ \
    fprintf (stderr, "Error: Unexpected input \"%s\".\n", input); \
    return -1; \
} while (0)



int parse_asm (FILE *source)
{
    uint16_t address = 0;

    char buffer [80] = { '\0' };

    uint8_t dst;
    uint8_t src;
    uint16_t value;

    rewind (source);

    while (fscanf (source, "%79s", buffer) != EOF)
    {
        /* Comment */
        if (strcmp ("/*", buffer) == 0)
        {
            do
            {
                SCAN_NEXT_TOKEN ();
            } while (strcmp ("*/", buffer) != 0);
        }

        /* Offset */
        else if (strncmp (".", buffer, 1) == 0)
        {
            PARSE_HEX_INT (address, & buffer [1]);
        }

        /* Label */
        else if (buffer [strlen (buffer) - 1] == ':')
        {
            buffer [strlen (buffer) - 1] = '\0';
            if (label_set_address (buffer, address) == -1)
            {
                return -1;
            }
        }

        /* mov */
        else if (strcmp ("mov", buffer) == 0)
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
            else
            {
                UNEXPECTED (buffer);
            }
        }

        /* ldi */
        else if (strcmp ("ldi", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* ldi rX, 0xXX */
            if (strncmp ("r", buffer, 1) == 0)
            {
                PARSE_REG (dst, buffer);

                SCAN_NEXT_TOKEN ();
                PARSE_INT (value, buffer);
                rom [address++] = LDI_R1_XX + (dst << 0);
                rom [address++] = (uint8_t) value;
            }
            /* ldi hl, 0xXXXX */
            else if (strncmp ("hl", buffer, 2) == 0)
            {
                uint16_t value = 0;

                SCAN_NEXT_TOKEN ();
                PARSE_INT (value, buffer);
                rom [address++] = LDI_HL_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
            else
            {
                UNEXPECTED (buffer);
            }
        }

        /* ld */
        else if (strcmp ("ld", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            if (strncmp ("r", buffer, 1) == 0)
            {
                PARSE_REG (dst, buffer);

                SCAN_NEXT_TOKEN ();

                /* ld rX, dc */
                if (strcmp ("dc", buffer) == 0)
                {
                    rom [address++] = LD_R1_DC + (dst << 0);
                }
                /* ld rX, [dh, rX] */
                else if (strncmp ("[dh", buffer, 3) == 0)
                {
                    SCAN_NEXT_TOKEN ();
                    PARSE_REG (src, buffer);
                    rom [address++] = LD_R1_DH_R1 + (dst << 2) + (src << 0);
                }
                /* ld rX, [0xXXXX] */
                else if (strncmp ("[", buffer, 1) == 0)
                {
                    SCAN_NEXT_TOKEN ();
                    PARSE_INT (value, buffer);
                    rom [address++] = LD_R1_XXXX + (dst << 0);
                    rom [address++] = (uint8_t) (value >> 8);
                    rom [address++] = (uint8_t) value;
                }
                else
                {
                    UNEXPECTED (buffer);
                }
            }
            else
            {
                UNEXPECTED (buffer);
            }
        }

        /* st */
        else if (strcmp ("st", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* st dc, rX */
            if (strcmp ("dc", buffer) == 0)
            {
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = ST_DC_R1 + (src << 0);
            }
            /* st [dh, rX], rx */
            else if (strncmp ("[dh", buffer, 3) == 0)
            {
                SCAN_NEXT_TOKEN ();
                PARSE_REG (dst, buffer);
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = ST_DH_R1_R1 + (dst << 2) + (src << 0);
            }
            /* st [0xXXXX], rX */
            else if (strncmp ("[", buffer, 1) == 0)
            {
                PARSE_INT (value, & buffer [1]);
                SCAN_NEXT_TOKEN ();
                PARSE_REG (src, buffer);
                rom [address++] = ST_XXXX_R1+ (src << 0);
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
            else
            {
                UNEXPECTED (buffer);
            }
        }

        /* pop rX */
        else if (strcmp ("pop", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            rom [address++] = POP_R1 + (dst << 0);
        }

        /* push rX */
        else if (strcmp ("push", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = PUSH_R1 + (src << 0);
        }

        /* jmp */
        else if (strcmp ("jmp", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* jmp hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = JMP_HL;
            }
            /* jmp 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = JMP_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }
        else if (strcmp ("jmp-z", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* jmp-z hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = JMP_Z_HL;
            }
            /* jmp-z 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = JMP_Z_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }
        else if (strcmp ("jmp-nz", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* jmp-nz hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = JMP_NZ_HL;
            }
            /* jmp-nz 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = JMP_NZ_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }
        else if (strcmp ("jmp-neg", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* jmp-neg hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = JMP_NEG_HL;
            }
            /* jmp-neg 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = JMP_NEG_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }
        else if (strcmp ("jmp-pos", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* jmp-pos hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = JMP_POS_HL;
            }
            /* jmp-pos 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = JMP_POS_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }

        /* call */
        else if (strcmp ("call", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();

            /* call hl */
            if (strcmp ("hl", buffer) == 0)
            {
                rom [address++] = CALL_HL;
            }
            /* call 0xXXXX */
            else
            {
                PARSE_INT (value, buffer);
                rom [address++] = CALL_XXXX;
                rom [address++] = (uint8_t) (value >> 8);
                rom [address++] = (uint8_t) value;
            }
        }

        /* ret */
        else if (strcmp ("ret", buffer) == 0)
        {
            rom [address++] = RET;
        }

        /* TODO: I/O */
        /* TODO cfg-set / cfg-clr */

        /* add */
        else if (strcmp ("add", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = ADD_R1_R1 + (dst << 2) + (src << 0);
        }

        /* addc */
        else if (strcmp ("addc", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = ADDC_R1_R1 + (dst << 2) + (src << 0);
        }

        /* sub */
        else if (strcmp ("sub", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = SUB_R1_R1 + (dst << 2) + (src << 0);
        }

        /* subc */
        else if (strcmp ("subc", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = SUBC_R1_R1 + (dst << 2) + (src << 0);
        }

        /* and */
        else if (strcmp ("and", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = AND_R1_R1 + (dst << 2) + (src << 0);
        }

        /* or */
        else if (strcmp ("or", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = OR_R1_R1 + (dst << 2) + (src << 0);
        }

        /* xor */
        else if (strcmp ("xor", buffer) == 0)
        {
            SCAN_NEXT_TOKEN ();
            PARSE_REG (dst, buffer);
            SCAN_NEXT_TOKEN ();
            PARSE_REG (src, buffer);
            rom [address++] = XOR_R1_R1 + (dst << 2) + (src << 0);
        }

        /* TODO: lshift */
        /* TODO: rshift */

        else
        {
#if 0
            UNEXPECTED (buffer);
#endif
            fprintf (stderr, "Warning: Unexpected input \"%s\".\n", buffer);
        }
    }
}

int find_labels (FILE *source)
{
    char *line = NULL;
    char name[80] = { '\0' };
    size_t n = 0;
    int match = 0;

    while (getline(& line, & n, source) != -1)
    {
        match = sscanf (line, "%79s", name);

        if (name[0] != '\0' && name [strlen (name) - 1] == ':')
        {
            name [strlen (name) - 1] = '\0';
            printf ("Found label: \"%s\".\n", name);
            if (label_add (name) == -1)
            {
                return -1;
            }
        }

        free (line);
        line = NULL;
        name[0] = '\0';
    }

    return 0;
}

int substitute_labels (void)
{
    return -1;
}

int cleanup_labels (void)
{
    for (int i = 0; i < label_count; i++)
    {
        if (labels [i]->reference_count != 0)
        {
            free (labels [i]->references);
        }
        free (labels [i]->name);
        free (labels [i]);
    }
    free (labels);
}

int main (int argc, char **argv)
{
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

    if (find_labels (source) == -1)
    {
        return EXIT_FAILURE;
    }

    if (parse_asm (source) == -1)
    {
        return EXIT_FAILURE;
    }

    if (substitute_labels () == -1)
    {
        return EXIT_FAILURE;
    }

    if (cleanup_labels () == -1)
    {
        return EXIT_FAILURE;
    }

    /* Write the rom to a file for the simulator */
    if (output_digital () == -1)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
