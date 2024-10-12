EXTRA_CFLAGS=-Wall
obj-m=driver.o 

KDIR=/lib/modules/`uname -r`/build 

all:
	make -C $(KDIR) M=`pwd` modules 
	gcc -o app app.c
clean:
	make -C $(KDIR) M=`pwd` clean
	rm -rf app