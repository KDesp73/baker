#include "writer.h"
#include <stdio.h>
#define CLI_IMPLEMENTATION
#include "extern/cli.h"
#include "extern/defer.h"
#include "version.h"


int main(int argc, char** argv)
{
    cli_args_t args = cli_args_make(
        cli_arg_new('h', "help", "Prints this message", no_argument), 
        cli_arg_new('v', "version", "Prints the current version and exits", no_argument), 
        NULL
    );
    defer { cli_args_free(&args); }

    int opt;
    LOOP_ARGS(opt, args) {
        switch (opt) {
            case 'h':
                cli_help(args, "baker <PATH> [<OPTIONS>...]", "Written by KDesp73");
                exit(0);
            case 'v':
                printf("baker v%s\n", VERSION_STRING);
            default:
                exit(1);
        } 
    }

    char* path = argv[argc - 1];
    if(argc == 1 || !path) {
        fprintf(stderr, "Provide a path\n");
        exit(1);
    }

    BakeOptions bakeopts = DefaultOpts(path);
    BakeFile(&bakeopts);

    printf("Baked %s into %s %s as symbol '%s'\n", bakeopts.input, bakeopts.c_out, bakeopts.h_out, bakeopts.symbol);
    BakeOptionsFree(&bakeopts);

    exit(0);
}
