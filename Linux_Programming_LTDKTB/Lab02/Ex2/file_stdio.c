#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int choice;
    char srcFile[100], destFile[100], buffer[1024];
    size_t bytesRead;

    printf("Select option:\n1. Enter from keyboard\n2. Copy from another file\nYour choice: ");
    scanf("%d", &choice);
    getchar();

    printf("Enter name of destination file: ");
    scanf("%s", destFile);
    getchar();

    FILE *dest = fopen(destFile, "w");
    if (!dest) {
        perror("Error creating file");
        return 1;
    }

    if (choice == 1) {
        printf("Enter content (end with Ctrl+D):\n");
        while ((bytesRead = fread(buffer, 1, sizeof(buffer), stdin)) > 0) {
            fwrite(buffer, 1, bytesRead, dest);
        }
    }
    else if (choice == 2) {
        printf("Enter source file name: ");
        scanf("%s", srcFile);

        FILE *src = fopen(srcFile, "r");
        if (!src) {
            perror("Error opening source file");
            fclose(dest);
            return 1;
        }

        while ((bytesRead = fread(buffer, 1, sizeof(buffer), src)) > 0) {
            fwrite(buffer, 1, bytesRead, dest);
        }
        fclose(src);
    } else {
        printf("Invalid choice.\n");
    }

    fclose(dest);
    printf("File created successfully.\n");
    return 0;
}
