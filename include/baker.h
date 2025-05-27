#ifndef BAKER_H
#define BAKER_H

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

#endif // BAKER_H
