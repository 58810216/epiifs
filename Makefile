ifneq ($(KERNELRELEASE),)
obj-m := hello.o

else
KDIR := /usr/src/linux-headers-2.6.31-14-server

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
endif
