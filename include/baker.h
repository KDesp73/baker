#ifndef BAKER_H
#define BAKER_H

/*--------------.
| Baker         |
`--------------*/

#define BAKER_VERSION_MAJOR 0
#define BAKER_VERSION_MINOR 1
#define BAKER_VERSION_PATCH 0
#define BAKER_VERSION "0.1.0"

typedef struct {
    char* input;
    char* symbol;
    char* c_out;
    char* h_out;
} BakeOptions;
BakeOptions DefaultOpts(const char* input);
void BakeOptionsFree(BakeOptions* opts);

#define ModifyOutput(opts, field, ext, target) \
    do { \
        free((opts).field); \
        char *_tmp_path = replace_ext((opts).input, (ext)); \
        (opts).field = move_to_dir(_tmp_path, (target)); \
        free(_tmp_path); \
    } while (0)

void BakeFile(BakeOptions* opts);
void BakeDirectory(const char *input_dir, BakeOptions *opts);
void BakeDirectorySingle(const char *input_dir, BakeOptions* opts);


/*--------------.
| File          |
|     Handling  |
`--------------*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>

bool is_file(const char* path);
bool is_directory(const char* path);
int write_file_bytes(const char *path, const unsigned char *data, size_t size);
unsigned char* read_file_bytes(const char *path, size_t *size);
char* replace_ext(const char *filename, const char *new_ext);
char* extract_filename(const char *path);
char* move_to_dir(const char *filepath, const char *new_dir);

#endif // BAKER_H
