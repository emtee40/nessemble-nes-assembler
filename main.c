#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "nessemble.h"

// path
char *cwd_path = NULL;

// cli
int flags = 0;

// io
unsigned int *rom = NULL;
unsigned int pass = 1;
int offset_max = 0;

// banks
int prg_offsets[MAX_BANKS];
int chr_offsets[MAX_BANKS];
int prg_index = 0;
int chr_index = 0;

// segment
char *segment = NULL;
int segment_type = 0;

// trainer
unsigned int trainer[TRAINER_MAX];
int offset_trainer = 0;

// symbols
int symbol_index = 0;
int rsset = 0;

// input
unsigned int length_ints = 0;
unsigned int ints[MAX_INTS];

// ines
struct ines_header ines = { 1, 0, 0, 1, 0 };

/**
 * Main function
 * @param {int} argc - Argument count
 * @param {char *} argv[] - Argument array
 */
int main(int argc, char *argv[]) {
    int rc = RETURN_OK, i = 0, l = 0, byte = 0;
    char *exec = NULL, *filename = NULL, *outfilename = NULL, *recipe = NULL;
    FILE *file = NULL, *outfile = NULL;

    // exec
    exec = argv[0];

    // usage
    if (argc < 2) {
        rc = usage(exec);
        goto cleanup;
    }

    // parse args
    for (i = 1, l = argc; i < l; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            rc = usage(exec);
            goto cleanup;
        }

        if (strcmp(argv[i], "-u") == 0 || strcmp(argv[i], "--undocumented") == 0) {
            flags |= FLAG_UNDOCUMENTED;
            continue;
        }

        if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
            if (i + 1 < l) {
                outfilename = argv[i + 1];
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--format") == 0) {
            if (i + 1 < l) {
                if (strcmp(argv[i+1], "nes") == 0 || strcmp(argv[i+1], "NES") == 0) {
                    flags |= FLAG_NES;
                }
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--disassemble") == 0) {
            flags |= FLAG_DISASSEMBLE;
            continue;
        }

        if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--simulate") == 0) {
            flags |= FLAG_SIMULATE;
            continue;
        }

        if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--recipe") == 0) {
            if (i + 1 < l) {
                recipe = argv[i+1];
            } else {
                rc = usage(exec);
                goto cleanup;
            }

            i += 1;
            continue;
        }

        filename = argv[i];
    }

    // get cwd
    if (!realpath(filename, cwd)) {
        fprintf(stderr, "Could not find real path of %s\n", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    cwd_path = strdup(cwd);

    for (i = (int)strlen(cwd_path), l = 0; i >= l; i--) {
        if (cwd_path[i] == '/') {
            cwd_path[i] = '\0';
            break;
        }
    }

    // simulate
    if (is_flag_simulate() == TRUE) {
        rc = simulate(cwd, recipe);
        goto cleanup;
    }

    // output
    if (!outfilename || strcmp(outfilename, "-") == 0) {
        outfilename = "/dev/stdout";
    }

    // disassemble
    if (is_flag_disassemble() == TRUE) {
        rc = disassemble(cwd, outfilename);
        goto cleanup;
    }

    // open file
    file = fopen(cwd, "r");

    if (!file) {
        fprintf(stderr, "Could not open %s\n", filename);

        rc = RETURN_EPERM;
        goto cleanup;
    }

    yyin = file;

    // segment
    segment = (char *)malloc(sizeof(char) * 8);
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    // offsets
    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    /* PASS 1 */

    pass = 1;

    do {
        (void)yyparse();
    } while (feof(yyin) == 0 && pass == 1);

    /* PASS 2 */

    // restart
    if (fseek(yyin, 0, SEEK_SET) != 0) {
        fprintf(stderr, "Seek error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // get offset max
    if (is_flag_nes() == TRUE) {
        offset_max = (ines.prg * BANK_PRG) + (ines.chr * BANK_CHR);
    }

    // create rom
    rom = (unsigned int *)malloc(sizeof(unsigned int) * offset_max);

    if (!rom) {
        fprintf(stderr, "Memory error\n");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    // set all bytes to 0xFF
    for (i = 0, l = offset_max; i < l; i++) {
        rom[i] = 0xFF;
    }

    // set all trainer bytes to 0xFF
    for (i = 0, l = TRAINER_MAX; i < l; i++) {
        trainer[i] = 0xFF;
    }

    // clear offsets
    offset_trainer = 0;

    // segments
    strcpy(segment, "PRG0");
    segment_type = SEGMENT_PRG;

    // offsets
    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    // offsets
    prg_index = 0;
    chr_index = 0;

    for (i = 0, l = MAX_BANKS; i < l; i++) {
        prg_offsets[i] = 0x00;
        chr_offsets[i] = 0x00;
    }

    // reset lineno
    yylineno = 1;

    do {
        (void)yyparse();
    } while (feof(yyin) == 0 && pass == 2);

    /* DONE */

    // write output
    outfile = fopen(outfilename, "w");

    // write nes header
    if (is_flag_nes() == TRUE) {
        (void)fwrite("NES", 3, 1, outfile); // 0-2

        byte = 0x1A;
        (void)fwrite(&byte, 1, 1, outfile); // 3

        (void)fwrite(&ines.prg, 1, 1, outfile); // 4
        (void)fwrite(&ines.chr, 1, 1, outfile); // 5

        byte = 0;
        byte |= ines.mir & 0x01;
        byte |= (ines.trn & 0x01) << 0x02;
        byte |= (ines.map & 0x0F) << 0x04;

        (void)fwrite(&byte, 1, 1, outfile); // 6

        byte = 0;
        byte |= (ines.map & 0xF0);

        (void)fwrite(&byte, 1, 1, outfile); // 7

        byte = 0;

        (void)fwrite(&byte, 1, 1, outfile); // 8
        (void)fwrite(&byte, 1, 1, outfile); // 9
        (void)fwrite(&byte, 1, 1, outfile); // 10

        for (i = 11, l = 16; i < l; i++) {
            (void)fwrite(&byte, 1, 1, outfile); // 11-15
        }
    }

    // write trainer
    if (ines.trn == 1) {
        for (i = 0, l = TRAINER_MAX; i < l; i++) {
            (void)fwrite(trainer+i, 1, 1, outfile);
        }
    }

    // write rom data
    for (i = 0, l = offset_max; i < l; i++) {
        (void)fwrite(rom+i, 1, 1, outfile);
    }

cleanup:
    if (file) {
        fclose(file);
    }

    return rc;
}

/**
 * Test if undocumented flag is active
 * @return {int} True if flag active, false if not
 */
int is_flag_undocumented() {
    return (flags & FLAG_UNDOCUMENTED) != 0 ? TRUE : FALSE;
}

/**
 * Test if nes flag is active
 * @return {int} True if flag active, false if not
 */
int is_flag_nes() {
    return (flags & FLAG_NES) != 0 ? TRUE : FALSE;
}

/**
 * Test if disassemble flag is active
 * @return {int} Return code
 */
int is_flag_disassemble() {
    return (flags & FLAG_DISASSEMBLE) != 0 ? TRUE : FALSE;
}

/**
 * Test if sumlate flag is active
 * @return {int} Return code
 */
int is_flag_simulate() {
    return (flags & FLAG_SIMULATE) != 0 ? TRUE : FALSE;
}

/**
 * Test if CHR segment
 * @return {int} True if is segment, false if not
 */
int is_segment_chr() {
    return segment_type == SEGMENT_CHR ? TRUE : FALSE;
}

/**
 * Test if PRG segment
 * @return {int} True if is segment, false if not
 */
int is_segment_prg() {
    return segment_type == SEGMENT_PRG ? TRUE : FALSE;
}

/**
 * Parser error
 * @param {const char *} fmt - Format string
 * @param {...} ... - Variable arguments
 */
void yyerror(const char *fmt, ...) {
    int rc = RETURN_EPERM;

    va_list argptr;
    va_start(argptr, fmt);

    fprintf(stderr, "Error on line %d: ", yylineno);
    (void)vfprintf(stderr, fmt, argptr);
    fprintf(stderr, "\n");

    va_end(argptr);

    exit(rc);
}