#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define CMDSIZ 32

void process_command(char *cmdbuf);

int main(int argc, char *argv[]) {
    int logout = 0, cmdsiz;
    char cmdbuf[CMDSIZ];

    while (!logout) {
        write(1, "myshell> ", 9);
        cmdsiz = read(0, cmdbuf, CMDSIZ);

        if (cmdsiz <= 1)
            continue;

        cmdbuf[cmdsiz - 1] = '\0';

        if (strcmp("logout", cmdbuf) == 0)
            ++logout;
        else
            process_command(cmdbuf);
    }

    printf("Exiting myshell...\n");
    return 0;
}

void process_command(char *cmdbuf) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return;
    }

    if (pid == 0) {
        // Child process: execute the command
        printf("Executing command: %s\n", cmdbuf);
        execlp(cmdbuf, cmdbuf, NULL);
        perror("Command execution failed");
        exit(1);
    } else {
        // Parent process: wait for child
        wait(NULL);
    }
}