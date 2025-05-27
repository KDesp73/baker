#include "baker.h"
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

void BakeDirectorySingle(const char *input_dir, BakeOptions* opts)
{
    char h_path[1024], c_path[1024];
    snprintf(h_path, sizeof(h_path), "%s/%s.h", opts->h_out, input_dir);
    snprintf(c_path, sizeof(c_path), "%s/%s.c", opts->c_out, input_dir);

    FILE *hout = fopen(h_path, "w");
    FILE *cout = fopen(c_path, "w");
    if (!hout || !cout) {
        perror("fopen");
        return;
    }

    fprintf(hout, "#pragma once\n\n");
    fprintf(hout, "#include <stddef.h>\n\n");
    fprintf(hout, "typedef struct {\n    const char *name;\n    const unsigned char *data;\n    size_t size;\n} BakedAsset;\n\n");
    fprintf(hout, "extern const BakedAsset baked_assets[];\n");
    fprintf(hout, "extern const size_t baked_assets_count;\n");

    fprintf(cout, "#include \"%s.h\"\n\n", input_dir);

    char **symbols = NULL;
    char **names = NULL;
    size_t *sizes = NULL;
    size_t file_count = 0;

    char cmd[512];
    snprintf(cmd, sizeof(cmd), "find %s -type f", input_dir);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) {
        perror("popen");
        fclose(hout);
        fclose(cout);
        return;
    }

    char filepath[1024];
    while (fgets(filepath, sizeof(filepath), pipe)) {
        filepath[strcspn(filepath, "\n")] = 0;

        size_t size;
        unsigned char *data = read_file_bytes(filepath, &size);
        if (!data) {
            fprintf(stderr, "Failed to read %s\n", filepath);
            continue;
        }

        char *symbol_full = strdup(filepath + strlen(input_dir));
        char *symbol = symbol_full;
        if (symbol[0] == '/' || symbol[0] == '\\') symbol++;
        sanitize(symbol);

        fprintf(cout, "static const unsigned char %s[%zu] = {", symbol, size);
        for (size_t i = 0; i < size; i++) {
            if (i % 12 == 0) fprintf(cout, "\n    ");
            fprintf(cout, "0x%02X,", data[i]);
        }
        fprintf(cout, "\n};\n");
        fprintf(cout, "static const size_t %s_len = %zu;\n\n", symbol, size);

        symbols = realloc(symbols, sizeof(char*) * (file_count + 1));
        names   = realloc(names, sizeof(char*) * (file_count + 1));
        sizes   = realloc(sizes, sizeof(size_t) * (file_count + 1));

        symbols[file_count] = strdup(symbol);
        names[file_count] = strdup(filepath + strlen(input_dir) + 1);
        sizes[file_count] = size;
        file_count++;

        free(symbol_full);
        free(data);
    }
    pclose(pipe);

    fprintf(cout, "const BakedAsset baked_assets[] = {\n");
    for (size_t i = 0; i < file_count; i++) {
        fprintf(cout, "    {\"%s\", %s, %zu},\n", names[i], symbols[i], sizes[i]);
        free(symbols[i]);
        free(names[i]);
    }
    fprintf(cout, "};\n");
    fprintf(cout, "const size_t baked_assets_count = %zu;\n", file_count);

    free(symbols);
    free(names);
    free(sizes);

    fclose(hout);
    fclose(cout);
}
