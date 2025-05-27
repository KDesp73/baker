#include "writer.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void sanitize_symbol(const char *filename, char *symbol)
{
    while (*filename) {
        if ((*filename >= 'a' && *filename <= 'z') ||
            (*filename >= 'A' && *filename <= 'Z') ||
            (*filename >= '0' && *filename <= '9')) {
            *symbol++ = *filename;
        } else {
            *symbol++ = '_';
        }
        filename++;
    }
    *symbol = '\0';
}

BakeOptions DefaultOpts(const char* input)
{
    char* symbol = (char*) malloc(256);
    sanitize_symbol(input, symbol);
    return (BakeOptions) {
        .input = strdup(input),
        .symbol = symbol,
        .c_out = replace_ext(input, ".c"),
        .h_out = replace_ext(input, ".h"),
    };
}

void BakeOptionsFree(BakeOptions* opts)
{
    if(opts->c_out)  free(opts->c_out);
    if(opts->h_out)  free(opts->h_out);
    if(opts->input)  free(opts->input);
    if(opts->symbol) free(opts->symbol);
}

void BakeFile(BakeOptions* opts)
{
    size_t size;
    unsigned char *data = read_file_bytes(opts->input, &size);
    if (!data) {
        fprintf(stderr, "Failed to read %s\n", opts->input);
        return;
    }

    const char *input_filename = extract_filename(opts->input);

    FILE *hout = fopen(opts->h_out, "w");
    FILE *cout = fopen(opts->c_out, "w");

    fprintf(hout, "#pragma once\n\n");
    fprintf(hout, "#include <stddef.h>\n\n");
    fprintf(hout, "extern const unsigned char %s[%zu];\n", opts->symbol, size);
    fprintf(hout, "extern const size_t %s_len;\n", opts->symbol);
    fprintf(hout, "extern const char *%s_name;\n", opts->symbol);

    fprintf(cout, "#include \"%s\"\n\n", extract_filename(opts->h_out));
    fprintf(cout, "const unsigned char %s[%zu] = {", opts->symbol, size);
    for (size_t i = 0; i < size; i++) {
        if (i % 12 == 0) fprintf(cout, "\n    ");
        fprintf(cout, "0x%02X,", data[i]);
    }
    fprintf(cout, "\n};\n");
    fprintf(cout, "const size_t %s_len = %zu;\n", opts->symbol, size);
    fprintf(cout, "const char *%s_name = \"%s\";\n", opts->symbol, input_filename);

    fclose(hout);
    fclose(cout);
    free(data);
}
