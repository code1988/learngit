KERNELDIR ?= /home/share/linux-2.6.32.2
all:memdev_test

memdev_test:memdev_test.c
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

clean:
	rm memdev_test
