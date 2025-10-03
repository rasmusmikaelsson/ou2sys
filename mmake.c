#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "parser.h"

#define FALSE 0;
#define TRUE 1;

void handel_target(const char *target, makefile *mmakefile);
void build_target(const char *target, char **args);

#define ERORR "\e[0;31m";
#define RESET "\e[0m";

int main(int argc, char *argv[]) {
    // int silenceCommands = FALSE;
    int forceBuild = FALSE;
    makefile *mmakefile;
	const char **rulePrereq;
    const char *defaultTarget;
	rule *currentRule;
	char **args;

    FILE *fp;
    int opt;

    while((opt = getopt(argc, argv, "f:Bs")) != -1) {
        switch (opt) {
            case 'f':
                printf("Found flag 'f' and filename %s\n", optarg);
                fp = fopen(optarg, "r");
                printf("Opened file: %s\n", optarg);
                break;
            case 'B':
                printf("Found flag 'B'\n");
                forceBuild = TRUE;
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

    printf("%p\n",fp);
    mmakefile = parse_makefile(fp);
    if(mmakefile != NULL) {
        defaultTarget = makefile_default_target(mmakefile);
		currentRule = makefile_rule(mmakefile, defaultTarget);
		rulePrereq = rule_prereq(currentRule);
		args = rule_cmd(currentRule);   
		printf("Default target: %s\n", defaultTarget);

		if(forceBuild) {
			// run arguments given for default target 
		} else {
			handel_target(defaultTarget, mmakefile);
		}

    } else {
        printf("Makefile = NULL\n");
    }
    return 0;
}

void handel_target(const char *target, makefile *mmakefile) {
	printf("current target: %s\n", target);
	rule *currentRule = makefile_rule(mmakefile, target);
	char **args = rule_cmd(currentRule);
	
	if(currentRule == NULL) {
		printf("last rule\n");
		return;
	}
	
	int index = 0;
	const char **current_rule_prereq = rule_prereq(currentRule);
	while(current_rule_prereq[index] != NULL) {
		handel_target(current_rule_prereq[index], mmakefile);

		build_target(current_rule_prereq[index], args);
		index++;
	}
}

void build_target(const char *target, char **args) {
	printf("Building target: %s\n", target);
	
}
