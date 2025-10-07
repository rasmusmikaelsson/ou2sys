#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <time.h>
#include "parser.h"

#define FALSE 0;
#define TRUE 1;

int handle_target(const char *target, makefile *mmakefile, int fB, int sC, FILE *fp);
int rebuild(char **args, int sC);
int file_exists(const char *target);
int updated_prereq(const char *target, const char **rule_prereq);
int exec_args(char **args);

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

int handle_target(const char *target, makefile *mmakefile, int fB, int sC, FILE *fp) {
	rule *currentRule = makefile_rule(mmakefile, target);
	
	if(currentRule == NULL) {
		if(!file_exists(target)) {
			fprintf(stderr, "%s: is not a file\n", target);
			return 1;
		}
		return 0;
	}
	
	const char **current_rule_prereq = rule_prereq(currentRule);

	// Loop current rules prereqs and call itself
	int index = 0;
	while(current_rule_prereq[index] != NULL) {
		if(handle_target(current_rule_prereq[index], mmakefile, fB, sC, fp) == 1) {
			return 1;
		}
		index++;
	}
	int is_updated_prereq = updated_prereq(target, current_rule_prereq);
	if(is_updated_prereq == 2) {
		return 1;
	}

	// Build project based params
	char **args = rule_cmd(currentRule);
	if(!file_exists(target) || fB || is_updated_prereq) {
		if(rebuild(args, sC) == 1) {
			return 1;
		}
	}
	return 0;
}

int rebuild(char **args, int sC) {
	pid_t pid;
	int status;
	
	if(!sC) {
		int index = 0;
		while(args[index] != NULL) {
			printf("%s", args[index]);
			if(args[index + 1] != NULL) {
				printf(" ");
			}
		index++;
		}
		printf("\n");
	}

	pid = fork();
	if(pid < 0) {
		perror("Fork failed");
		return 1;
	} else if(pid == 0) {
		if(exec_args(args) == 1) {
			return 1;
		}
	}

	wait(&status);

	if(WIFEXITED(status) && WEXITSTATUS(status) != EXIT_SUCCESS) {
		return 1;
	}
	return 0;
}

int exec_args(char **args) {
	if(execvp(args[0], args) == -1) {
		perror("execvp failed");
		return 1;
	}
	return 0;
}

// Checks if a given target is a file
int file_exists(const char *target) {	
	FILE *check_file = fopen(target, "r");
	if(check_file) {
		fclose(check_file);
		return 1;
	}
	return 0;
}	

int updated_prereq(const char *target, const char **rule_prereq) {
	struct stat target_mtime;
	struct stat prereq_mtime;
	
	if(!file_exists(target)) {
		return 1;
	}
	
	if(stat(target, &target_mtime) == -1) {
		printf("target stat failed\n");
		perror("stat failed");
		return 2;
	}
	
	int index = 0;
	while(rule_prereq[index] != NULL) {
		if(!file_exists(rule_prereq[index])) {
			return 1;
		}
		if(stat(rule_prereq[index], &prereq_mtime) == -1) {
			printf("prereq stat failed\n");
			perror("stat failed");
			return 2;
		}

		if(target_mtime.st_mtim.tv_sec < prereq_mtime.st_mtim.tv_sec) {
			return 1;
		}
		index++;
	}
	return 0;
}
