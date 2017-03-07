#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "nessemble.h"

#define CONFIG_FILENAME    "config"
#define CONFIG_LINE_COUNT  10
#define CONFIG_LINE_LENGTH 256
#define CONFIG_TYPES       1

struct config_type {
    char *type;
};

struct config_type config_types[CONFIG_TYPES] = {
    { "registry" }
};

unsigned int open_config(FILE **file, char **filename) {
    unsigned int rc = RETURN_OK;
    char *config_path = NULL;
    size_t length = 0;
    struct passwd *pw = getpwuid(getuid());
    FILE *config = NULL;
    DIR *dir = NULL;

    if (!pw) {
        error_program_log("Could not find home directory");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    length = strlen(pw->pw_dir) + 18;
    config_path = (char *)nessemble_malloc(sizeof(char) * length + 1);

    sprintf(config_path, "%s/%s", pw->pw_dir, "." PROGRAM_NAME);

    dir = opendir(config_path);

    if (!dir) {
        if (mkdir(config_path, 0777) != 0) {
            rc = RETURN_EPERM;
            goto cleanup;
        }
    } else {
        (void)closedir(dir);
    }

    strcat(config_path, "/" CONFIG_FILENAME);

    if (access(config_path, F_OK) == -1) {
        config = fopen(config_path, "w+");
    } else {
        config = fopen(config_path, "r+");
    }

    if (!config) {
        error_program_log("Could not open `" CONFIG_FILENAME "`");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    *file = config;
    *filename = config_path;

cleanup:
    return rc;
}

void close_config(FILE *file, char *filename) {
    if (file) {
        (void)fclose(file);
    }

    nessemble_free(filename);
}

unsigned int get_config(char **result, char *item) {
    unsigned int rc = RETURN_OK, found = FALSE;
    char line[CONFIG_LINE_LENGTH], key[CONFIG_LINE_LENGTH], val[CONFIG_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    while (fgets(line, CONFIG_LINE_LENGTH, config) != NULL) {
        if (sscanf(line, "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            found = TRUE;
            *result = nessemble_strdup(val);
        }
    }

    if (found == FALSE) {
        error_program_log("No `%s` set", item);

        rc = RETURN_EPERM;
    }

cleanup:
    close_config(config, config_path);

    return rc;
}

unsigned int set_config(char *result, char *item) {
    unsigned int rc = RETURN_OK, found = FALSE, line_index = 0, i = 0, l = 0;
    char lines[CONFIG_LINE_COUNT][CONFIG_LINE_LENGTH], key[CONFIG_LINE_LENGTH], val[CONFIG_LINE_LENGTH];
    char *config_path = NULL;
    FILE *config = NULL;

    if ((rc = open_config(&config, &config_path)) != RETURN_OK) {
        goto cleanup;
    }

    while (fgets(lines[line_index], CONFIG_LINE_LENGTH, config) != NULL) {
        if (sscanf(lines[line_index++], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            found = TRUE;
        }
    }

    if (found != TRUE) {
        (void)fclose(config);
        config = fopen(config_path, "a");

        if (!config) {
            error_program_log("Could not open `" CONFIG_FILENAME "`");

            rc = RETURN_EPERM;
            goto cleanup;
        }

        fprintf(config, "%s\t%s\n", item, result);

        goto cleanup;
    }

    (void)fclose(config);
    config = fopen(config_path, "w+");

    if (!config) {
        error_program_log("Could not open `" CONFIG_FILENAME "`");

        rc = RETURN_EPERM;
        goto cleanup;
    }

    for (i = 0, l = line_index; i < l; i++) {
        if (sscanf(lines[i], "%s %s\n", key, val) != 2) {
            continue;
        }

        if (strcmp(key, item) == 0) {
            fprintf(config, "%s\t%s\n", key, result);
        } else {
            fprintf(config, "%s\t%s\n", key, val);
        }
    }

cleanup:
    close_config(config, config_path);

    return rc;
}

unsigned int list_config(char **result) {
    unsigned int rc = RETURN_OK, longest = 0, length = 0, i = 0, l = 0;
    char *output = NULL, *val = NULL;

    for (i = 0, l = CONFIG_TYPES; i < l; i++) {
        length = (unsigned int)strlen(config_types[i].type);

        if (length > longest) {
            longest = length + 1;
        }
    }

    length = (longest + CONFIG_LINE_LENGTH + 2) * CONFIG_TYPES;
    output = (char *)nessemble_malloc(sizeof(char) * (length + 1));

    for (i = 0, l = CONFIG_TYPES; i < l; i++) {
        if ((rc = get_config(&val, config_types[i].type)) != RETURN_OK) {
            goto cleanup;
        }

        sprintf(output+strlen(output), "%s%*s%s\n", config_types[i].type, longest - (unsigned int)strlen(config_types[i].type), " ", val);
        nessemble_free(val);
    }

    output[strlen(output)-1] = '\0';

cleanup:
    *result = output;

    return rc;
}