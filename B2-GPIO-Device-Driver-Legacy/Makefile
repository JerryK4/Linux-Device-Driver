.PHONY:all clean

obj-m +=gpio.o 	#Tên file source 
EXTRA_CFLAGS=-Wall

CROSS:=/home/jerry/working_space/Jerry/BBB/bb-kernel/dl/gcc-8.5.0-nolibc/arm-linux-gnueabi/bin/arm-linux-gnueabi- #Trỏ vào toolchain
KERNEL:=/home/jerry/working_space/Jerry/BBB/bb-kernel/KERNEL #Trỏ vào đường dẫn tuyệt đối chưa kernel source 
all:
	make ARCH=arm CROSS_COMPILE=$(CROSS) -C $(KERNEL) M=$(shell pwd) modules
clean:
	make -C $(KERNEL) M=$(shell pwd) clean
