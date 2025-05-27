#include "baker.h"
#include "files.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

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


static void sanitize(char *s)
{
    for (; *s; s++) {
        if (*s == '/' || *s == '.' || *s == '-') *s = '_';
    }
}

static void walk_and_bake(const char *base_dir, const char *rel_path, BakeOptions *opts)
{
    char path[1024];
    snprintf(path, sizeof(path), "%s/%s", base_dir, rel_path);

    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;

        char next_rel[1024];
        snprintf(next_rel, sizeof(next_rel), "%s/%s", rel_path, entry->d_name);

        char full_path[1024];
        snprintf(full_path, sizeof(full_path), "%s/%s", base_dir, next_rel);

        struct stat st;
        if (stat(full_path, &st) != 0) continue;

        if (S_ISDIR(st.st_mode)) {
            walk_and_bake(base_dir, next_rel, opts);
        } else if (S_ISREG(st.st_mode)) {
            char *symbol = strdup(next_rel);
            sanitize(symbol);

            BakeOptions local = *opts;
            local.input = strdup(full_path);
            local.symbol = symbol;

            char *tmp_h = replace_ext(entry->d_name, ".h");
            char *tmp_c = replace_ext(entry->d_name, ".c");

            local.h_out = move_to_dir(tmp_h, opts->h_out);
            local.c_out = move_to_dir(tmp_c, opts->c_out);

            BakeFile(&local);

            free(local.input);
            free(local.symbol);
            free(local.h_out);
            free(local.c_out);
            free(tmp_h);
            free(tmp_c);
        }
    }

    closedir(dir);
}

void BakeDirectory(const char *input_dir, BakeOptions *opts)
{
    walk_and_bake(input_dir, "", opts);
}
