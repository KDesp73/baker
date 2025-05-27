#ifndef WRITER_H
#define WRITER_H

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


#endif // WRITER_H
