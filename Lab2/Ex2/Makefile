obj-m := hello_param.o
KDIR := $(HOME)/linux-5.15.163
PWD := $(shell pwd)
all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean