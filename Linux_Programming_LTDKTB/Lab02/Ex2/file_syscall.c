#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int choice;
    char srcFile[100], destFile[100], buffer[1024];
    ssize_t bytesRead;

    printf("Select option:\n1. Enter from keyboard\n2. Copy from another file\nYour choice: ");
    scanf("%d", &choice);
    getchar(); // bỏ ký tự '\n' còn lại

    printf("Enter name of destination file: ");
    scanf("%s", destFile);
    getchar();

    int fdDest = open(destFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fdDest < 0) {
        perror("Error creating file");
        return 1;
    }

    if (choice == 1) {
        printf("Enter content (end with Ctrl+D):\n");
        while ((bytesRead = read(STDIN_FILENO, buffer, sizeof(buffer))) > 0) {
            write(fdDest, buffer, bytesRead);
        }
    }
    else if (choice == 2) {
        printf("Enter source file name: ");
        scanf("%s", srcFile);

        int fdSrc = open(srcFile, O_RDONLY);
        if (fdSrc < 0) {
            perror("Error opening source file");
            close(fdDest);
            return 1;
        }

        while ((bytesRead = read(fdSrc, buffer, sizeof(buffer))) > 0) {
            write(fdDest, buffer, bytesRead);
        }
        close(fdSrc);
    } else {
        printf("Invalid choice.\n");
    }

    close(fdDest);
    printf("File created successfully.\n");
    return 0;
}
