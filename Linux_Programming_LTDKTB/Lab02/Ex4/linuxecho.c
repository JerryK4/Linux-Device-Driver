#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s [string] [filename]\n", argv[0]);
        return 1;
    }

    const char *text = argv[1];
    const char *filename = argv[2];

    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    if (write(fd, text, strlen(text)) == -1) {
        perror("Error writing to file");
        close(fd);
        return 1;
    }

    write(fd, "\n", 1); // thêm xuống dòng giống echo
    close(fd);

    printf("Wrote '%s' to %s successfully.\n", text, filename);
    return 0;
}