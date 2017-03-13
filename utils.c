#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "nessemble.h"

#ifdef WIN32
#include <windows.h>
#endif /* WIN32 */

#ifndef WIN32
#include <string.h>
char *strstr(const char *haystack, const char *needle);
#define _GNU_SOURCE
#include <string.h>
char *strcasestr(const char *haystack, const char *needle);
char *realpath(const char *path, char *resolved_path);
#endif /* WIN32 */

void *nessemble_malloc(size_t size) {
    void *mem = NULL;

    mem = malloc(sizeof(char) * size);

    if (!mem) {
        fatal("Memory error");
    }

    memset(mem, 0, size);

    return mem;
}

void nessemble_free(void *ptr) {
    if (ptr) {
        free(ptr);
    }
}

void nessemble_fclose(FILE *file) {
    if (file) {
        fclose(file);
    }
}

char *nessemble_strdup(char *str) {
    char *dup = NULL;
    size_t length = 0;

    length = strlen(str);
    dup = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    strcpy(dup, str);

    return dup;
}

const char *nessemble_strcasestr(const char *s1, const char *s2) {
#ifdef WIN32
    return strstr(s1, s2);
#else /* WIN32 */
    return strcasestr(s1, s2);
#endif
}

int nessemble_mkdir(const char *dirname, int mode) {
    int rc = 0;

#ifdef WIN32
    rc = mkdir(dirname);
#else /* WIN32 */
    rc = mkdir(dirname, mode);
#endif /* WIN32 */

    return rc;
}

char *nessemble_getpass(const char *prompt) {
    char *pass = NULL;

#ifdef WIN32
    size_t length = 0;

    pass = (char *)nessemble_malloc(sizeof(char) * 256);

    fputs(prompt, stdout);

    // hide cursor
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);

    while (get_line(&pass, NULL) != NULL) {
        length = strlen(pass);

        if (length - 1 == 0) {
            continue;
        }

        pass[length - 1] = '\0';
        break;
    }

    info.bVisible = TRUE;
    SetConsoleCursorInfo(consoleHandle, &info);
#else /* WIN32 */
    pass = getpass(prompt);
#endif

    return pass;
}

char *nessemble_realpath(const char *path, char *resolved_path) {
#ifdef WIN32
    int rc = RETURN_OK;
    char *lppPart = NULL;

    if ((rc = GetFullPathName(path, 1024, resolved_path, &lppPart)) == 0) {
        return NULL;
    }

    return resolved_path;
#else /* WIN32 */
    return realpath(path, resolved_path);
#endif /* WIN32 */
}

unsigned int is_stdout(char *filename) {
    unsigned int rc = FALSE;
    FILE *file = NULL;

    file = fopen(filename, "w+");

    if (!file) {
        goto cleanup;
    }

    if (isatty(fileno(file)) == 1 || strcmp(filename, "/dev/stdout") == 0) {
        rc = TRUE;
    }

cleanup:
    nessemble_fclose(file);

    return rc;
}

/**
 * Convert hex string to int
 * @param {char *} hex - Hexadecimal string (ex: $12)
 * @return {int} Base-10 integer
 */
int hex2int(char *hex) {
    size_t length = strlen(hex);

    if (hex[length - 1] == 'h') {
        hex[length - 1] = '\0';
    }

    return (int)strtol(hex, NULL, 16);
}

/**
 * Convert binary string to int
 * @param {char *} bin - Binary string (ex: %110)
 * @return {int} Base-10 integer
 */
int bin2int(char *bin) {
    size_t length = strlen(bin);

    if (bin[length - 1] == 'b') {
        bin[length - 1] = '\0';
    }

    return (int)strtol(bin, NULL, 2);
}

/**
 * Convert octal string to int
 * @param {char *} oct - Octal string
 * @return {int} Base-10 integer
 */
int oct2int(char *oct) {
    size_t length = strlen(oct);

    if (oct[length - 1] == 'o') {
        oct[length - 1] = '\0';
    }

    return (int)strtol(oct, NULL, 8);
}

/**
 * Convert decimal string to int
 * @param {char *} dec - Decimal string (ex: 34)
 * @return {int} Base-10 integer
 */
int dec2int(char *dec) {
    size_t length = strlen(dec);

    if (dec[length - 1] == 'd') {
        dec[length - 1] = '\0';
    }

    return (int)strtol(dec, NULL, 10);
}

/**
 * Convert defchr string to int
 * @param {char *} dec - defchr string (ex: 123..321)
 * @return {int} Base-10 integer
 */
int defchr2int(char *defchr) {
    unsigned int i = 0, l = 0;
    size_t length = strlen(defchr);

    for (i = 0, l = (unsigned int)length; i < l; i++) {
        if (defchr[i] == '.') {
            defchr[i] = '0';
        }
    }

    return dec2int(defchr);
}

unsigned int fgetu16_little(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);

    return a | (b << 8);
}

unsigned int fgetu16_big(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);

    return (a << 8) | b;
}

unsigned int fgetu32_little(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);
    unsigned int c = (unsigned int)fgetc(fp);
    unsigned int d = (unsigned int)fgetc(fp);

    return a | (b << 8) | (c << 16) | (d << 24);
}

unsigned int fgetu32_big(FILE *fp) {
    unsigned int a = (unsigned int)fgetc(fp);
    unsigned int b = (unsigned int)fgetc(fp);
    unsigned int c = (unsigned int)fgetc(fp);
    unsigned int d = (unsigned int)fgetc(fp);

    return (a << 24) | (b << 16) | (c << 8) | d;
}

/**
 * Hash string
 * @param {char *} string - String to hash
 * @return {unsigned int} Hash
 */
unsigned int str2hash(char *string) {
    unsigned int hash = 5381, i = 0, l = 0;
    size_t length = strlen(string);

    for (i = 1, l = (unsigned int)length - 1; i < l; i++) {
        hash = ((hash << 5) + hash) + (unsigned int)string[i];
    }

    return hash;
}

/**
 * Base64 encode
 * @param {char **} encoded - Encoded string
 * @param {char *} str - Input string
 */
unsigned int base64enc(char **encoded, char *str) {
    unsigned int rc = RETURN_OK;
    unsigned int u = 0, len = 0, w = 0, i = 0, index = 0;
    size_t length = 0;
	char c[4];
    char *output = NULL;
    const char *alpha = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    length = strlen(str);
    output = (char *)nessemble_malloc(sizeof(char) * length);

    memset(c, '\0', 4);
    memset(output, '\0', length);

	do {
		c[1] = c[2] = 0;

        memcpy(c, str+i, 4);

        if ((len = strlen(c)) >= 3) {
            len = 3;
            i += 3;
        }

		u = c[0] << 16 | c[1] << 8 | c[2];

        output[index++] = alpha[u >> 18];
        output[index++] = alpha[(u >> 12) & 63];
        output[index++] = len < 2 ? '=' : alpha[u >> 6 & 63];
        output[index++] = len < 3 ? '=' : alpha[u & 63];

		if (++w == 19) {
            w = 0;
        }
	} while (len == 3);

    *encoded = output;

	return rc;
}

/**
 * Get fullpath
 */
int get_fullpath(char **path, char *string) {
    int rc = RETURN_OK;
    size_t string_length = 0, path_length = 0;
    char *fullpath = NULL;

    if (string[0] == '<') {
        if ((rc = get_libpath(path, string)) != RETURN_OK) {
            error("Could not include library `%s`", string);
        }

        goto cleanup;
    }

    string_length = strlen(string);
    path_length = strlen(cwd_path) + string_length - 1;
    fullpath = (char *)nessemble_malloc(sizeof(char) * (path_length + 1));

    strcpy(fullpath, cwd_path);
    strcat(fullpath, "/");
    strncat(fullpath, string + 1, string_length - 2);

    if (fullpath[path_length-1] == '"') {
        fullpath[path_length-1] = '\0';
    }

    *path = fullpath;

cleanup:
    return rc;
}

/**
 * Get libpath
 */
int get_libpath(char **path, char *string) {
    int rc = RETURN_OK;
    size_t string_length = 0, path_length = 0;
    char *home = NULL, *fullpath = NULL;

    if ((rc = get_home(&home)) != RETURN_OK) {
        goto cleanup;
    }

    string_length = strlen(string);
    path_length = strlen(home) + 11 + string_length - 1;
    fullpath = (char *)nessemble_malloc(sizeof(char) * (path_length + 1));

    strcpy(fullpath, home);
    strcat(fullpath, "/." PROGRAM_NAME "/");
    strncat(fullpath, string + 1, string_length - 2);

    if (fullpath[path_length-1] == '>') {
        fullpath[path_length-1] = '\0';
    }

    if (access(fullpath, F_OK) == -1) {
        rc = RETURN_EPERM;
    }

    *path = fullpath;

cleanup:
    nessemble_free(home);

    return rc;
}

/**
 * Load file
 */
unsigned int load_file(char **data, char *filename) {
    unsigned int insize = 0;
    char *indata = NULL;
    FILE *infile = NULL;

    infile = fopen(filename, "r");

    if (!infile) {
        error_program_log("Could not open `%s`", filename);
        goto cleanup;
    }

    if (fseek(infile, 0, SEEK_END) != 0) {
        error_program_log("Seek error");
        goto cleanup;
    }

    insize = (unsigned int)ftell(infile);

    if (fseek(infile, 0, SEEK_SET) != 0) {
        error_program_log("Seek error");

        insize = 0;
        goto cleanup;
    }

    if (insize == 0) {
        error_program_log("`%s` is empty", filename);

        insize = 0;
        goto cleanup;
    }

    indata = (char *)nessemble_malloc(sizeof(char) * (insize + 1));

    if (fread(indata, 1, (size_t)insize, infile) != (size_t)insize) {
        error_program_log("Could not read `%s`", filename);

        insize = 0;
        goto cleanup;
    }

    *data = indata;

cleanup:
    nessemble_fclose(infile);

    return insize;
}

unsigned int tmp_save(FILE *file, char *filename) {
    unsigned int rc = RETURN_OK;
    char ch = '\0';
    FILE *tmp = NULL;

    tmp = fopen(filename, "w+");

    if (!tmp) {
        rc = RETURN_EPERM;
        goto cleanup;
    }

    while (fread(&ch, 1, 1, file) > 0) {
        UNUSED(fwrite(&ch, 1, sizeof(ch), tmp));
    }

cleanup:
    nessemble_fclose(tmp);

    return rc;
}

void tmp_delete(char *filename) {
    UNUSED(unlink(filename));
}

char *get_line(char **buffer, char *prompt) {
    if (prompt) {
        printf("%s", prompt);
    }

    return fgets(*buffer, 256, stdin);
}

/**
 * Test if undocumented flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_undocumented() {
    return (unsigned int)((flags & FLAG_UNDOCUMENTED) != 0 ? TRUE : FALSE);
}

/**
 * Test if nes flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_nes() {
    return (unsigned int)((flags & FLAG_NES) != 0 ? TRUE : FALSE);
}

/**
 * Test if disassemble flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_disassemble() {
    return (unsigned int)((flags & FLAG_DISASSEMBLE) != 0 ? TRUE : FALSE);
}

/**
 * Test if reassemble flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_reassemble() {
    return (unsigned int)((flags & FLAG_REASSEMBLE) != 0 ? TRUE : FALSE);
}

/**
 * Test if sumlate flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_simulate() {
    return (unsigned int)((flags & FLAG_SIMULATE) != 0 ? TRUE : FALSE);
}

/**
 * Test if check flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_check() {
    return (unsigned int)((flags & FLAG_CHECK) != 0 ? TRUE : FALSE);
}

/**
 * Test if coverage flag is active
 * @return {unsigned int} True if flag active, false if not
 */
unsigned int is_flag_coverage() {
    return (unsigned int)((flags & FLAG_COVERAGE) != 0 ? TRUE : FALSE);
}

/**
 * Test if CHR segment
 * @return {unsigned int} True if is segment, false if not
 */
unsigned int is_segment_chr() {
    return (unsigned int)(segment_type == SEGMENT_CHR ? TRUE : FALSE);
}

/**
 * Test if PRG segment
 * @return {unsigned int} True if is segment, false if not
 */
unsigned int is_segment_prg() {
    return (unsigned int)(segment_type == SEGMENT_PRG ? TRUE : FALSE);
}
