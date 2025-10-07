/**
 * main.c - Entry point for the simplified make utility.
 *
 * This program parses a Makefile-like file and determines which
 * targets need to be rebuilt based on their dependencies and timestamps.
 * It supports options to force rebuilds, silence commands, and specify
 * custom makefiles.
 * 
 * Synopsis:
 *      ./mmake [-f MAKEFILE] [-B] [-s] [TARGET...]
 *
 * Options:
 *      -f MAKEFILE : Use a custom makefile instead of the default "mmakefile".
 *      -B          : Force rebuild all targets, regardless of timestamps.
 *      -s          : Silence command output to stdout.
 *
 * Targets:
 *      One or more specific targets to build. If no targets are provided,
 *      the program builds the default target defined in the makefile.
 *
 * Author: Rasmus
 * Date: 2025-10-07
 */

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

/**
 * Entry point of the program. This function parses command-line arguments, opens and parses the
 * specified makefile, and determines which targets should be built.
 *
 * @param argc	Argument count
 * @param argv	Argument vector
 * @return		0 on success, non-zero on error
 */
int main(int argc, char *argv[]) {
    int sC = FALSE;			// Silence commands
    int fB = FALSE;			// Force build
    makefile *mmakefile;
	const char *filename = "mmakefile";
    const char *defaultTarget;

    FILE *fp;
    int opt;

	// Parse commandline options
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

	// Open a specified makefile, or open default
	fp = fopen(filename, "r");
	if(fp == NULL) {
		fprintf(stderr, "No such file or directory\n");
		exit(EXIT_FAILURE);
	}

	// Parse the makefile
	if((mmakefile = parse_makefile(fp)) == NULL) {
		fprintf(stderr, "%s: Could not parse makefile\n", filename);
		fclose(fp);
		exit(EXIT_FAILURE);
	} 

	// Handel specified target, or default target
    if(mmakefile != NULL) {
		int target_specified = FALSE;

		// Handel specified targets
		for(int i = optind; i < argc ; i++) {
			target_specified = TRUE;
			if(handle_target(argv[i], mmakefile, fB, sC, fp) == 1) {
				makefile_del(mmakefile);
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}

		// If no specified targets, build the default target
        if(!target_specified) {
			defaultTarget = makefile_default_target(mmakefile);
			if(handle_target(defaultTarget, mmakefile, fB, sC, fp) == 1) {
				makefile_del(mmakefile);
				fclose(fp);
				exit(EXIT_FAILURE);
			}
		}
    }

	// Cleanup and exit successfully
	makefile_del(mmakefile);
	fclose(fp);
    return 0;
}
