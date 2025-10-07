#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "target.h"

/* ------------------ Declarations of internal functions ------------------ */

static int updated_prereq(const char *target, const char **rule_prereq);
static int exec_args(char **args);
static int file_exists(const char *target);
static int rebuild_target(char **args, int sC);

/* -------------------------- Internal functions -------------------------- */

/**
 * Executes a targets given arguments.
 * 
 * @param args	argument list
 * @return		0 if arguments was successfully executed, oterwise 1
 */
static int exec_args(char **args) {
	if(execvp(args[0], args) == -1) {
		perror("execvp failed");
		return 1;
	}
	return 0;
}

// Checks if a given target is a file
static int file_exists(const char *target) {	
	FILE *check_file = fopen(target, "r");
	if(check_file) {
		fclose(check_file);
		return 1;
	}
	return 0;
}

static int updated_prereq(const char *target, const char **rule_prereq) {
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

/**
 * Rebuilds a given target. If -s (silence commands), the executed
 * arguments will not be shown in stdout.
 *
 *  @param args	argument list
 *  @param sC	silence commands defaults to false, otherwise true
 *  @return		0 if the target is rebuilt successfully, otherwise 1
 */
static int rebuild_target(char **args, int sC) {
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



/* -------------------------- External functions -------------------------- */

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
		if(rebuild_target(args, sC) == 1) {
			return 1;
		}
	}
	return 0;
}

