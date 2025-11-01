#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // open()
#include <unistd.h>     // read(), close(), lseek()
#include <errno.h>
#include <string.h>

int main() {
    int fd;
    char buffer[256];
    ssize_t bytesRead;

    // --- a) Đọc toàn bộ nội dung ---
    fd = open("Hello.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    printf("a) Full content of Hello.txt:\n");
    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("Error reading file");
        close(fd);
        return 1;
    }
    buffer[bytesRead] = '\0';  // Thêm ký tự kết thúc chuỗi
    printf("%s\n", buffer);

    close(fd);

    // --- b) Đọc từ byte thứ 7 ---
    fd = open("Hello.txt", O_RDONLY);
    if (fd < 0) {
        perror("Error opening file");
        return 1;
    }

    // Di chuyển con trỏ file đến vị trí thứ 7 (tính từ đầu)
    if (lseek(fd, 7, SEEK_SET) == -1) {
        perror("Error seeking in file");
        close(fd);
        return 1;
    }

    printf("\nb) Content from 7th byte:\n");
    bytesRead = read(fd, buffer, sizeof(buffer) - 1);
    if (bytesRead < 0) {
        perror("Error reading file");
        close(fd);
        return 1;
    }
    buffer[bytesRead] = '\0';
    printf("%s\n", buffer);

    close(fd);
    return 0;
}
