ifneq ($(KERNELRELEASE),)
obj-m := epiifs.o
epiifs-objs := super.o disk-io.o 

else
KDIR := /usr/src/linux-headers-2.6.31-14-server

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	rm -f *.ko *.o *.mod.o *.mod.c *.symvers
	rm -f *.markers 
	rm -f modules.order
endif
