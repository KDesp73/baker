#include "files.h"
#include "baker.h"
#include <bits/getopt_core.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#define CLI_IMPLEMENTATION
#include "extern/cli.h"
#include "extern/defer.h"
#include "version.h"


int main(int argc, char** argv)
{
    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "Prints this message", no_argument), 
        cli_arg_new('v', "version", "Prints the current version and exits", no_argument), 
        cli_arg_new('H', "h-out", "Specify the header output directory", required_argument),
        cli_arg_new('C', "c-out", "Specify the source output directory", required_argument),
        cli_arg_new('s', "single", "Bake a directory of assets into a single .c/.h pair", no_argument),
        NULL
    );
    defer { cli_args_free(&args); }

    char* h_out = NULL;
    char* c_out = NULL;
    bool single = false;

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                cli_help(args, "baker <PATH> [<OPTIONS>...]", "Written by KDesp73");
                exit(0);
            case 'v':
                printf("baker v%s\n", VERSION_STRING);
                exit(0);
            case 'H':
                h_out = optarg;
                break;
            case 'C':
                c_out = optarg;
                break;
            case 's':
                single = true;
                break;
            default:
                exit(1);
        } 
    }

    char* path = argv[argc - 1];
    if(argc == 1 || !path) {
        fprintf(stderr, "Provide a path\n");
        exit(1);
    }

    if(is_file(path)) {
        BakeOptions bakeopts = DefaultOpts(path);
        ModifyOutput(bakeopts, h_out, ".h", (h_out) ? h_out : "include");
        ModifyOutput(bakeopts, c_out, ".c", (c_out) ? c_out : "src");
        BakeFile(&bakeopts);

        printf("Baked %s into %s %s as symbol '%s'\n", bakeopts.input, bakeopts.c_out, bakeopts.h_out, bakeopts.symbol);
        BakeOptionsFree(&bakeopts);
    } else if(is_directory(path)) {
        BakeOptions bakeopts = {
            .h_out = (h_out) ? h_out : "include",
            .c_out = (c_out) ? c_out : "src",
        };

        (single)
            ? BakeDirectorySingle(path, &bakeopts)
            : BakeDirectory(path, &bakeopts);
    }

    exit(0);
}
