#ifndef FILES_H
#define FILES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static inline unsigned char* read_file_bytes(const char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;

    fseek(f, 0, SEEK_END);
    *size = ftell(f);
    rewind(f);

    unsigned char *data = malloc(*size);
    if (fread(data, 1, *size, f) != *size) {
        fclose(f);
        free(data);
        return NULL;
    }

    fclose(f);
    return data;
}

static inline char *replace_ext(const char *filename, const char *new_ext)
{
    const char *dot = strrchr(filename, '.');
    size_t base_len;

    if (dot) {
        base_len = dot - filename;
    } else {
        base_len = strlen(filename);
    }

    size_t new_len = base_len + strlen(new_ext) + 1;
    char *result = malloc(new_len);
    if (!result) return NULL;

    strncpy(result, filename, base_len);
    result[base_len] = '\0';
    strcat(result, new_ext);

    return result;
}

#endif // FILES_H
