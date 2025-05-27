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

void BakeFile(BakeOptions* opts);


#endif // WRITER_H
