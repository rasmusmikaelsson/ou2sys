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

int handle_target(const char *target, makefile *mmakefile, int fB, int sC);
void rebuild(const char *target, char **args, int sC, makefile *mmakefile);
int file_exists(const char *target);
int updated_prereq(const char *target, const char **rule_prereq, makefile *mmakefile);
void exec_args(char **args, makefile *mmakefile);

#define ERR "\e[0;31m";
#define GRN "\e[0;32m";
#define YEL "\e[0;33m";
#define RESET "\e[0m";


int main(int argc, char *argv[]) {
    int sC = FALSE;
    int fB = FALSE;
    makefile *mmakefile;
    const char *defaultTarget;

    FILE *fp = fopen("mmakefile", "r");
    int opt;

    while((opt = getopt(argc, argv, "f:Bs")) != -1) {
        switch (opt) {
            case 'f':
				fclose(fp);
                fp = fopen(optarg, "r");
                break;
            case 'B':
                fB = TRUE;
                break;
            case 's':
                sC = TRUE
                break;
            case '?':
                printf("Unknowed flag..\n");
                break;
            default:
                printf("Error\n");
                return -1;
        }
    }

    mmakefile = parse_makefile(fp);
    if(mmakefile != NULL) {
        defaultTarget = makefile_default_target(mmakefile);
		handle_target(defaultTarget, mmakefile, fB, sC);
    } else {
        printf("Makefile = NULL\n");
    }
	printf("deleting makefile\n");
	makefile_del(mmakefile);
	fclose(fp);
    return 0;
}

int handle_target(const char *target, makefile *mmakefile, int fB, int sC) {
	rule *currentRule = makefile_rule(mmakefile, target);
	
	if(currentRule == NULL) {
		if(!file_exists(target)) {
			printf("%s: is not a file\n", target);
			return 1;
		}
		return 0;
	}
	char **args = rule_cmd(currentRule);
	
	// Get current rules prereqs:
	const char **current_rule_prereq = rule_prereq(currentRule);

	// Loop current rules prereqs and call itself:
	int index = 0;
	while(current_rule_prereq[index] != NULL) {
		handle_target(current_rule_prereq[index], mmakefile, fB, sC);
		index++;
	}

	// Build prject based params
	if(!file_exists(target) || fB || updated_prereq(target, current_rule_prereq, mmakefile)) {
		rebuild(target, args, sC, mmakefile);
	}
	return 0;
}

void rebuild(const char *target, char **args, int sC, makefile *mmakefile) {
	pid_t pid;
	int status;
	
	if(!sC) {
		int index = 0;
		while(args[index] != NULL) {
			printf("%s ", args[index]);
			index++;
		}
		printf("\n");
	}

	pid = fork();
	if(pid < 0) {
		perror("Fork failed");
		makefile_del(mmakefile);
		exit(EXIT_FAILURE);
	} else if(pid == 0) {
		exec_args(args, mmakefile);
	}

	wait(&status);
}

void exec_args(char **args, makefile *mmakefile) {
	if(execvp(args[0], args) == -1) {
		perror("execvp failed");
		makefile_del(mmakefile);
		exit(EXIT_FAILURE);
	}
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

int updated_prereq(const char *target, const char **rule_prereq, makefile *mmakefile) {
	struct stat target_mtime;
	struct stat prereq_mtime;

	if(stat(target, &target_mtime) == -1) {
		perror("stat failed");
		makefile_del(mmakefile);
		exit(EXIT_FAILURE);
	}
	
	int index = 0;
	while(rule_prereq[index] != NULL) {
		if(stat(rule_prereq[index], &prereq_mtime) == -1) {
			perror("stat failed");
			makefile_del(mmakefile);
			exit(EXIT_FAILURE);
		}

		if(target_mtime.st_mtim.tv_sec < prereq_mtime.st_mtim.tv_sec) {
			return 1;
		}
		index++;
	}
	return 0;
}
