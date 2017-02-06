#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "nessemble.h"
#include "init.h"

static char *get_line(char **buffer, char *prompt) {
    printf("%s", prompt);
    return fgets(*buffer, BUF_SIZE, stdin);
}

unsigned int init() {
    unsigned int rc = RETURN_OK, i = 0, l = 0;
    unsigned int input_prg = 0, input_chr = 0, input_mapper = 0, input_mirroring = 0;
    size_t length = 0;
    char *buffer = NULL, *input_filename = NULL;
    FILE *output = NULL;

    buffer = malloc(sizeof(char) * BUF_SIZE);

    if (!buffer) {
        fprintf(stderr, "Memory error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (get_line(&buffer, "Filename: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';
        input_filename = strdup(buffer);
        break;
    }

    while (get_line(&buffer, "PRG Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%u", &input_prg) != 1) {
            continue;
        }

        break;
    }

    while (get_line(&buffer, "CHR Banks: ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%u", &input_chr) != 1) {
            continue;
        }

        break;
    }


    while (get_line(&buffer, "Mapper (0-255): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%u", &input_mapper) != 1) {
            continue;
        }

        break;
    }

    while (get_line(&buffer, "Mirroring (0-15): ") != NULL) {
        length = strlen(buffer);

        if (length - 1 == 0) {
            continue;
        }

        buffer[length - 1] = '\0';

        if (sscanf(buffer, "%u", &input_mirroring) != 1) {
            continue;
        }

        break;
    }

    if (!input_filename) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    output = fopen(input_filename, "w+");

    if (!output) {
        fprintf(stderr, "Could not open `%s`\n", input_filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    fprintf(output, ".inesprg %u\n.ineschr %u\n.inesmap %u\n.inesmir %u\n", input_prg, input_chr, input_mapper % 0xFF, input_mirroring % 0x0F);

    for (i = 0, l = input_prg; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.prg %u\n", i);

        if (i == 0) {
            init_asm[init_asm_len-1] = '\0';
            fprintf(output, "\n%s\n", (char *)init_asm);
        }
    }

    for (i = 0, l = input_chr; i < l; i++) {
        fprintf(output, "\n;;;;;;;;;;;;;;;;\n\n.chr %u\n", i);
    }

    printf("Created `%s`\n", input_filename);

cleanup:
    if (input_filename) {
        free(input_filename);
    }

    if (buffer) {
        free(buffer);
    }

    if (output) {
        (void)fclose(output);
    }

    return rc;
}