#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }
    else if (pid == 0) {
        // Child process
        printf("Child process (PID=%d): Executing 'ls -l'...\n", getpid());
        
        // Execute the command "ls -l" in the current directory
        execlp("ls", "ls", "-l", NULL);

        // If execlp() fails, this line will execute
        perror("execlp failed");
        exit(EXIT_FAILURE);
    }
    else {
        // Parent process
        printf("Parent process (PID=%d): Waiting for child to finish...\n", getpid());
        wait(NULL);
        printf("Parent process: Child process finished.\n");
    }

    return 0;
}