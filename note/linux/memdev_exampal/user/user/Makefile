KERNELDIR ?=/home/share/linux-2.6.32.2/include
all: read_block write_noblock 

read_block : read_block.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

write_noblock : write_noblock.c
	#arm-linux-gcc -I$(KERNELDIR) -s -Wl,-warn-common --static -o $@ $^
	arm-linux-gcc -I$(KERNELDIR) -o $@ $^

                                                                                                        
clean :
	rm read_block write_noblock
