#character device driver

EXTRA_CFLAGS=-Wall
obj-m=exam.o #Khi build exam.c -> exam.ko 

KDIR=/lib/modules/`uname -r`/build 

all:
	make -C $(KDIR) M=`pwd` modules 
clean:
	make -C $(KDIR) M=`pwd` clean