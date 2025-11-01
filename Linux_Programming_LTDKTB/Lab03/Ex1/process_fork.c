#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid;

    // Create a child process
    pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        exit(1);
    }

    // Child process
    if (pid == 0) {
        printf("Child process: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Child process is running...\n");
        sleep(3); // Simulate work
        printf("Child process finished.\n");
        exit(0);
    }
    // Parent process
    else {
        printf("Parent process: PID = %d\n", getpid());

        // (a) Parent does NOT wait
        printf("[a] Parent continues without waiting for child.\n");
        sleep(1);
        printf("[a] Parent finished early.\n\n");

        // (b) Parent waits for child
        printf("[b] Parent now waits for child to finish.\n");
        wait(NULL); // Wait for child process to complete
        printf("[b] Child has finished. Parent continues.\n");
    }

    return 0;
}