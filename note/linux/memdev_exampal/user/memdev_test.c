#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

int main()
{
	int fd;
	char buf[]="hello world!\n";
	fd_set rfds,wfds;
	
	fd = open("/dev/memdev0",O_RDWR | O_NONBLOCK);	
	while(1)
	{
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_SET(fd,&rfds);
		FD_SET(fd,&wfds);
	
		select(fd+1,&rfds,&wfds,NULL,NULL);
	
		if(FD_ISSET(fd,&rfds))
		{
			printf("poll moniter can be read\n");
			lseek(fd,0,SEEK_SET);
			read(fd,buf,sizeof(buf));
			printf("read value is %s\n",buf);	
			sleep(1);
		}
		if(FD_ISSET(fd,&wfds))
		{
			printf("poll moniter can be writed\n");
			lseek(fd,0,SEEK_SET);
			write(fd,buf,sizeof(buf));
			printf("write value is %s\n",buf);
			sleep(1);	
		}
	}
	return 0;
}
