#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include "parser.h"
#include "target.h"

#define FALSE 0;
#define TRUE 1;

#define ERR "\e[0;31m";
#define GRN "\e[0;32m";
#define YEL "\e[0;33m";
#define RESET "\e[0m";


int main(int argc, char *argv[]) {
    int sC = FALSE;
    int fB = FALSE;
    makefile *mmakefile;
	const char *filename = "mmakefile";
    const char *defaultTarget;

    FILE *fp;
    int opt;

    while((opt = getopt(argc, argv, "f:Bs")) != -1) {
        switch (opt) {
            case 'f':
				filename = optarg;
                break;
            case 'B':
                fB = TRUE;
                break;
            case 's':
                sC = TRUE;
                break;
            case '?':
                printf("Unknown flag..\n");
                break;
            default:
                printf("Error\n");
                return -1;
        }
    }

	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "No such file or directory\n");
		exit(EXIT_FAILURE);
	}

	if((mmakefile = parse_makefile(fp)) == NULL) {
		fprintf(stderr, "%s: Could not parse makefile\n", filename);
		fclose(fp);
		exit(EXIT_FAILURE);
	} 

    if(mmakefile != NULL) {
		int target_specified = 0;
		for(int i = optind; i < argc ; i++) {
			target_specified = 1;
			if(handle_target(argv[i], mmakefile, fB, sC, fp) == 1) {
				makefile_del(mmakefile);
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}

        if(!target_specified) {
			defaultTarget = makefile_default_target(mmakefile);
			if(handle_target(defaultTarget, mmakefile, fB, sC, fp) == 1) {
				makefile_del(mmakefile);
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}
    }

	makefile_del(mmakefile);
	fclose(fp);
    return 0;
}
