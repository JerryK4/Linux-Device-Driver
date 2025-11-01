#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

static int idata = 1000;   // Allocated in data segment

int main() {
    int istack = 150;      // Allocated in stack segment
    int fd = open("Hello.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open");
        exit(1);
    }

    pid_t childPid = fork();

    if (childPid < 0) {
        perror("fork failed");
        exit(1);
    }

    // ---------------- CHILD PROCESS ----------------
    if (childPid == 0) {
        printf("Child process (PID=%d):\n", getpid());
        printf("Before change - idata = %d, istack = %d\n", idata, istack);

        // Modify variables
        idata += 50;
        istack += 50;

        printf("After change  - idata = %d, istack = %d\n", idata, istack);

        // Move file offset to 7th byte from the beginning
        if (lseek(fd, 7, SEEK_SET) == -1) {
            perror("lseek failed");
        } else {
            write(fd, "ChildWasHere\n", 13);
        }

        close(fd);
        _exit(0);  // Exit child process
    }

    // ---------------- PARENT PROCESS ----------------
    else {
        wait(NULL); // Wait for child to finish

        printf("\nParent process (PID=%d):\n", getpid());
        printf("idata = %d, istack = %d\n", idata, istack);

        // Move file offset to beginning and read
        lseek(fd, 0, SEEK_SET);
        char buffer[128] = {0};
        int bytes = read(fd, buffer, sizeof(buffer) - 1);
        if (bytes >= 0) {
            printf("\nContent of Hello.txt:\n%s\n", buffer);
        } else {
            perror("read failed");
        }

        close(fd);
    }

    return 0;
}
