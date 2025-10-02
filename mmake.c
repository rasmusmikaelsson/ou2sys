#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "parser.h"

#define FALSE 0;
#define TRUE 1;

#define ERORR "\e[0;31m";
#define RESET "\e[0m";

int main(int argc, char *argv[]) {
    // int silenceCommands = FALSE;
    // int forceBuild = FALSE;
    makefile *mmakefile;
    const char *defaultTarget;

    FILE *fp;
    int opt;

    while((opt = getopt(argc, argv, "f:Bs")) != -1) {
        switch (opt) {
            case 'f':
                printf("Found flag 'f' and filename %s\n", optarg);
                fp = fopen(optarg, "r");
                break;
            case 'B':
                printf("Found flag 'B'\n");
                // forceBuild = TRUE;
                break;
            case 's':
                printf("Found flag 's'\n");
                // silenceCommands = TRUE
                break;
            case '?':
                printf("Unknowed flag..\n");
                break;
            default:
                printf("Error\n");
                return -1;
        }
    }

    printf("custom makefile = TRUE\n");
    printf("%p\n",fp);
    mmakefile = parse_makefile(fp);
    if(mmakefile != NULL) {
        defaultTarget = makefile_default_target(mmakefile);
        printf("Default target: %s", defaultTarget);
    } else {
        printf("Makefile = NULL\n");
    }

    return 0;
}
