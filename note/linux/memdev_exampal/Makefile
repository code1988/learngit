ifeq ($(KERNELRELEASE),)

KERNELDIR ?= /home/share/linux-2.6.32.2
PWD := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

modules-install:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules-install

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

.PHONY: modules modules-install clean

else
	obj-m := memdev.o
endif
