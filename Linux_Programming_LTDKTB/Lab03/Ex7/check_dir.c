#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>

volatile sig_atomic_t stop = 0; // Flag for stopping program when Ctrl+C is pressed

// Signal handler for Ctrl+C
void handle_sigint(int sig) {
    (void)sig; // avoid unused parameter warning
    stop = 1;
    printf("\nProgram is terminated by user\n");
}

// Function to list files in a directory
void list_files(const char *path) {
    DIR *dir;
    struct dirent *entry;
    struct stat fileStat;
    char fullPath[512];

    dir = opendir(path);
    if (dir == NULL) {
        perror("Unable to open directory");
        return;
    }

    printf("Listing files in directory: %s\n", path);
    printf("-------------------------------------\n");

    while ((entry = readdir(dir)) != NULL) {
        // Skip . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(fullPath, sizeof(fullPath), "%s/%s", path, entry->d_name);
        if (stat(fullPath, &fileStat) == 0) {
            if (S_ISREG(fileStat.st_mode)) {
                printf("File: %s (Size: %ld bytes)\n", entry->d_name, fileStat.st_size);
            } else if (S_ISDIR(fileStat.st_mode)) {
                printf("Dir : %s/\n", entry->d_name);
            }
        }
    }

    closedir(dir);
    printf("-------------------------------------\n");
}

int main(int argc, char *argv[]) {
    char *dirPath;

    if (argc < 2) {
        dirPath = ".";
    } else {
        dirPath = argv[1];
    }

    // Set up Ctrl+C signal handler
    signal(SIGINT, handle_sigint);

    while (!stop) {
        time_t now = time(NULL);
        printf("\n[%s]\n", ctime(&now));
        list_files(dirPath);

        // Sleep for 1 minute
        for (int i = 0; i < 60 && !stop; i++)
            sleep(1);
    }

    return 0;
}
