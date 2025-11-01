#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>

#define MAX_LINE 1024

// Hàm phụ: kiểm tra file có chứa từ khóa không
int file_contains_keyword(const char *filename, const char *keyword) {
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        return 0;

    char line[MAX_LINE];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        if (strstr(line, keyword)) {
            found = 1;
            break;
        }
    }

    fclose(fp);
    return found;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s [keyword] [directory]\n", argv[0]);
        return 1;
    }

    const char *keyword = argv[1];
    const char *dirpath = argv[2];
    struct dirent *entry;
    DIR *dp = opendir(dirpath);

    if (dp == NULL) {
        perror("Error opening directory");
        return 1;
    }

    printf("--- Files in '%s' containing '%s' ---\n", dirpath, keyword);

    while ((entry = readdir(dp)) != NULL) {
        // Bỏ qua thư mục "." và ".."
        if (entry->d_type == DT_REG) { // chỉ xử lý file thường
            char filepath[1024];
            snprintf(filepath, sizeof(filepath), "%s/%s", dirpath, entry->d_name);

            if (file_contains_keyword(filepath, keyword)) {
                printf("%s\n", entry->d_name);
            }
        }
    }

    closedir(dp);
    return 0;
}
