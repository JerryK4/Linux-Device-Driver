# Makefile đơn giản cho bài so sánh system call và stdio

all:
	gcc file_syscall.c -o file_syscall
	gcc file_stdio.c -o file_stdio

time:
	@echo "\n--- Testing system call version ---"
	time ./file_syscall
	@echo "\n--- Testing stdio version ---"
	time ./file_stdio

clean:
	rm -f file_syscall file_stdio
.PHONY: all time clean