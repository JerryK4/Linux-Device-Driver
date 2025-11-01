#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINE 1024

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [word] [filename]\n", argv[0]);
        return 1;
    }

    const char *keyword = argv[1];
    const char *filename = argv[2];
    FILE *fp = fopen(filename, "r");

    if (fp == NULL) {
        perror("Error opening file");
        return 1;
    }

    char line[MAX_LINE];
    int line_num = 1;
    int found = 0;

    printf("--- Lines containing '%s' ---\n", keyword);

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, keyword)) {
            printf("%d: %s", line_num, line);
            found = 1;
        }
        line_num++;
    }

    if (!found)
        printf("No matches found.\n");

    fclose(fp);
    return 0;
}
