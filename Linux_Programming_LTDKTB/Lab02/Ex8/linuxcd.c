#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>

int main(int argc, char *argv[]) {
    // Check if exactly one argument (directory path) is provided
    if (argc != 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    // Attempt to change the current working directory
    if (chdir(argv[1]) != 0) {
        perror("chdir failed");
        return 1;
    }

    // Get and display the new current working directory
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd failed");
        return 1;
    }

    printf("Changed directory successfully.\n");
    printf("Current directory: %s\n", cwd);
    return 0;
}