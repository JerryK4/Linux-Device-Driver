#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    int pipe1[2]; // pipe1: child → parent
    int pipe2[2]; // pipe2: parent → child
    pid_t pid;

    // Create both pipes
    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid = fork();

    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // -------------------
    // CHILD PROCESS
    // -------------------
    if (pid == 0) {
        char msg_to_parent[] = "Hello parent";
        char msg_from_parent[100];

        // Close unused pipe ends
        close(pipe1[0]); // child doesn't read from pipe1
        close(pipe2[1]); // child doesn't write to pipe2

        // Send message to parent
        write(pipe1[1], msg_to_parent, strlen(msg_to_parent) + 1);
        close(pipe1[1]); // close after sending

        // Read message from parent
        read(pipe2[0], msg_from_parent, sizeof(msg_from_parent));
        printf("Child received: %s\n", msg_from_parent);
        close(pipe2[0]);

        exit(0);
    }

    // -------------------
    // PARENT PROCESS
    // -------------------
    else {
        char msg_from_child[100];
        char msg_to_child[] = "Hello children";

        // Close unused pipe ends
        close(pipe1[1]); // parent doesn't write to pipe1
        close(pipe2[0]); // parent doesn't read from pipe2

        // Read message from child
        read(pipe1[0], msg_from_child, sizeof(msg_from_child));
        printf("Parent received: %s\n", msg_from_child);
        close(pipe1[0]);

        // Send message to child
        write(pipe2[1], msg_to_child, strlen(msg_to_child) + 1);
        close(pipe2[1]);

        wait(NULL); // wait for child to finish
    }

    return 0;
}
