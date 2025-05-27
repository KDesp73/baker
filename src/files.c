#include "baker.h"


int write_file_bytes(const char *path, const unsigned char *data, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 0;

    size_t written = fwrite(data, 1, size, f);
    fclose(f);
    return written == size;
}

unsigned char* read_file_bytes(const char *path, size_t *size)
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

char *replace_ext(const char *filename, const char *new_ext)
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

char *extract_filename(const char *path)
{
    const char *slash1 = strrchr(path, '/');   // Unix-like
    const char *slash2 = strrchr(path, '\\');  // Windows

    const char *last_sep = slash1 > slash2 ? slash1 : slash2;
    return last_sep ? (char *)(last_sep + 1) : (char *)path;
}

char *move_to_dir(const char *filepath, const char *new_dir)
{
    const char *filename = extract_filename(filepath);

    size_t dir_len = strlen(new_dir);
    size_t needs_slash = (new_dir[dir_len - 1] != '/' && new_dir[dir_len - 1] != '\\') ? 1 : 0;
    size_t full_len = dir_len + needs_slash + strlen(filename) + 1;

    char *result = malloc(full_len);
    if (!result) return NULL;

    strcpy(result, new_dir);
    if (needs_slash) strcat(result, "/");
    strcat(result, filename);

    return result;
}

bool is_file(const char* path) 
{
    struct stat st;
    return (stat(path, &st) == 0) && S_ISREG(st.st_mode);
}

bool is_directory(const char* path) 
{
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

