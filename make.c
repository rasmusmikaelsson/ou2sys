#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main(void) {
    char* command = "gcc";
    char* arguments_list[] = {"gcc", "hello.c", "-Wall", "-o", "test", NULL};

    if(fork() == 0) {
        // Child process
        printf("Executing command: %s\n", command);
        if(execvp(command, arguments_list) == -1) {
            fprintf(stderr, "Error executing: %s\n", strerror(errno));
            return EXIT_FAILURE;
        }
    } else {
        // Parent process
        printf("Parent process waiting for child to complete...\n");
        wait(NULL); // Wait for child process to finish
        printf("Child process completed.\n");
        return EXIT_SUCCESS;
    }
}