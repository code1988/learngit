#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

int main(int argc, char* argv[])
{
	int		ret, ret1, ret2;
	if (access("/usr/local/bak/appwin", F_OK) == 0) {
		ret = system("cp /usr/local/bak/appwin /usr/local/bin/appwin");
		if (ret == 0)
			system("rm /usr/local/bak/appwin");
	}
	if (access("/usr/local/bak/fsnpacket", F_OK) == 0) {
		ret = system("cp /usr/local/bak/fsnpacket /usr/local/bin/fsnpacket");
		if (ret == 0)
			system("rm /usr/local/bak/fsnpacket");
	}
	ret1 = 0;
	if (access("/usr/local/bin/appwin", F_OK))
		ret1 = 1;
	if (access("/usr/local/bin/fsnpacket", F_OK))
		ret1 = 1;
	if (ret1) {
		system("mkdir /media/sdcard");
		system("mount -t vfat /dev/mmcblk0p1 /media/sdcard");
		system("chmod +x /media/sdcard/upgrade/arm/appwin");
		system("/media/sdcard/upgrade/arm/appwin &");
		system("chmod +x /media/sdcard/upgrade/arm/fsnpacket");
		system("/media/sdcard/upgrade/arm/fsnpacket &");
	}
	else {
		system("chmod +x /usr/local/bin/appwin");
		system("/usr/local/bin/appwin &");
		system("chmod +x /usr/local/bin/fsnpacket");
		system("/usr/local/bin/fsnpacket &");
	}
		
	return 0;
}



