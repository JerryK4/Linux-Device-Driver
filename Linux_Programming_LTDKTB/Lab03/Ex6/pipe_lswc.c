#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    pid_t pid;

    // Create a pipe
    if (pipe(fd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    // -----------------------------
    // CHILD PROCESS: executes "ls"
    // -----------------------------
    if (pid == 0) {
        // Close read end of pipe (child only writes)
        close(fd[0]);

        // Redirect stdout to pipe's write end
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);

        // Execute "ls"
        execlp("ls", "ls", NULL);
        perror("execlp ls failed");
        exit(EXIT_FAILURE);
    }

    // -----------------------------
    // PARENT PROCESS: executes "wc -l"
    // -----------------------------
    else {
        // Close write end of pipe (parent only reads)
        close(fd[1]);

        // Redirect stdin to pipe's read end
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);

        // Execute "wc -l"
        execlp("wc", "wc", "-l", NULL);
        perror("execlp wc failed");
        exit(EXIT_FAILURE);
    }

    return 0;
}
