#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

int main() {
    char filename[256];
    struct stat fileStat;

    printf("Enter file name: ");
    scanf("%s", filename);

    // Gọi stat() để lấy thông tin file
    if (stat(filename, &fileStat) < 0) {
        perror("Error getting file information");
        return 1;
    }

    // In kích thước file
    printf("\n--- File Information ---\n");
    printf("File: %s\n", filename);
    printf("Size: %ld bytes\n", fileStat.st_size);

    // Kiểm tra quyền truy cập (Access mode)
    printf("Permissions: ");
    printf( (fileStat.st_mode & S_IRUSR) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWUSR) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXUSR) ? "x" : "-");
    printf( (fileStat.st_mode & S_IRGRP) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWGRP) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXGRP) ? "x" : "-");
    printf( (fileStat.st_mode & S_IROTH) ? "r" : "-");
    printf( (fileStat.st_mode & S_IWOTH) ? "w" : "-");
    printf( (fileStat.st_mode & S_IXOTH) ? "x\n" : "-\n");

    // In loại file
    printf("File type: ");
    if (S_ISREG(fileStat.st_mode))
        printf("Regular file\n");
    else if (S_ISDIR(fileStat.st_mode))
        printf("Directory\n");
    else if (S_ISLNK(fileStat.st_mode))
        printf("Symbolic link\n");
    else
        printf("Other\n");

    return 0;
}
