#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // Check if the user provides exactly one argument
    if (argc != 2) {
        printf("Usage: %s <directory_path>\n", argv[0]);
        return 1;
    }

    char *path = argv[1];
    DIR *dir = opendir(path);
    if (!dir) {
        perror("opendir failed");
        return 1;
    }

    struct dirent *entry;
    struct stat info;
    char fullpath[512];

    printf("--- Listing directory: %s ---\n", path);

    // Read all entries in the directory
    while ((entry = readdir(dir)) != NULL) {
        // Skip the special directories "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Combine directory path and file name
        snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

        // Get detailed file information
        if (stat(fullpath, &info) == 0) {
            printf("%-25s  %10ld bytes  ", entry->d_name, info.st_size);

            // Convert last modified time to readable format
            char timebuf[64];
            struct tm *t = localtime(&info.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", t);
            printf("%s\n", timebuf);
        }
    }

    closedir(dir);
    return 0;
}