/**
 * target.c - Handles building and rebuilding of Makefile targets.
 *
 * This file contains logic for determining whether a target or its
 * prerequisites need to be rebuilt, based on modification times and
 * user-specified build flags.
 *
 * Functions:
 *  - handle_target(): Handles recursive target checking and rebuild logic.
 *  - exec_args(): Executes a target's command arguments.
 *  - file_exists(): Checks if a target file exists.
 *  - updated_prereq(): Determines if any prerequisites are newer than the target.
 *  - rebuild_target(): Forks and executes rebuild commands for a target.
 *
 * @Author:		Rasmus Mikaelsson (et24rmn)
 * @Date:		2025-10-07
 * @Version:	1.0
 */

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

	// Loop through prerequisites and recursively handle each one
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

	// Build project based parameters
	char **args = rule_cmd(currentRule);
	if(!file_exists(target) || fB || is_updated_prereq) {
		if(rebuild_target(args, sC) == 1) {
			return 1;
		}
	}
	return 0;
}

/* -------------------------- Internal functions -------------------------- */

/**
 * Executes a given list of command arguments.
 * 
 * @param args	Argument list
 * @return		0 if arguments was successfully executed, oterwise 1
 */
static int exec_args(char **args) {
	if(execvp(args[0], args) == -1) {
		perror("execvp failed");
		return 1;
	}
	return 0;
}

/** 
 * Checks where or not a given target is a file
 *
 * @param target	Target to check
 * @return			0 if target is not a file, otherwise 1
 */
static int file_exists(const char *target) {	
	FILE *check_file = fopen(target, "r");
	if(check_file) {
		fclose(check_file);
		return 1;
	}
	return 0;
}

/**
 * Determines if any of the prerequisites is newer than the target.
 *
 * @param target		The target file
 * @param rule_prereq	List of the given rules prerequisites
 * @return				1 if a rebuild is needed, 0 if "up-to-date", and 
 *						2 if error occures
 */
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
 * Rebuilds a given target by executing it's commands.
 *
 *  @param args		Argument list for the rebuild command.
 *  @param sC		Silence flag. If true, suppresses command output.
 *  @return			0 if the target is rebuilt successfully, otherwise 1
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
