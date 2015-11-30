/*********************************************************************************************
#####         上海嵌入式家园-开发板商城         #####
#####                    www.embedclub.com                        #####
#####             http://embedclub.taobao.com               #####

* File：	read_noblock.c
* Author:	Hanson
* Desc：	test  Beep driver with wait queue
* History:	May 16th 2011
*********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/ioctl.h>

#define BEEP_MAGIC 'k'
#define BEEP_START_CMD _IO (BEEP_MAGIC, 1)
#define BEEP_STOP_CMD _IO (BEEP_MAGIC, 2)

int main()
{
	int i = 0;
	int dev_fd;
	char rdbuf[10];
//	dev_fd = open("/dev/beep",O_RDWR | O_NONBLOCK);
	dev_fd = open("/dev/memdev0",O_RDWR);
	
	if ( dev_fd == -1 ) {
		printf("Cann't open file /dev/memdev\n");
		exit(1);
	}
	//process block here.
	i = read(dev_fd,rdbuf,2);

	printf("read from device\n");
	
	close(dev_fd);
	return 0;
}
