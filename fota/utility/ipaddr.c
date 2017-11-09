/*
 *
 *  Filename        : ipaddr.c
 *  Version         : 1.0.0.0
 *  Author          : Xiaole
 *  Created         : 2009/5/18
 *  Description     :
 *
 *  History         :
 *  1.  Date        : 2009/5/18
 *      Author      : Xiaole
 *      Modification: Created file
 *		Descripter	: now this file only support linux
*/

/****************************************************************************/
/*								INCLUDE										*/
/****************************************************************************/
#if defined(WIN32)
#include <windows.h>
#include <winsock.h>
#endif
#if defined(LINUX)
#include <linux/sockios.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <netinet/in.h> 
#include <net/if_arp.h> 
#include <errno.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <netinet/ether.h>
#include <linux/mii.h>

#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#endif 

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "utypes.h"

/*
 *	Function :	ipaddr_set_f
 *	Describer:	set ip address
 *	Parameter:	ifname			if interface name
 *				ip				ip address
 *				mask			ip mask
 *				gate			ip gate
 *	Return   :	OK_T			success
 *				ERROR_T			failed
*/
status_t ifaddr_set_f(s8_t* ifname, s32_t ip, s32_t mask, s32_t gate)
{
	s8_t	buf[128], buf1[32], buf2[32];
	struct in_addr	addr1, addr2;

	memcpy(&addr1, &ip, 4);
	memcpy(&addr2, &mask, 4);
	strcpy(buf1, inet_ntoa(addr1));
	strcpy(buf2, inet_ntoa(addr2));
	sprintf(buf, "ifconfig %s %s netmask %s", ifname, buf1, buf2);
	system(buf);
	memcpy(&addr1, &gate, 4);
	strcpy(buf1, inet_ntoa(addr1));
	sprintf(buf, "route add default gw %s", buf1);
	system(buf);

	return OK_T;
}

status_t ifmac_set_f(s8_t *ifname, s8_t *mac)
{
	
	return OK_T;
}

status_t ifmac_get_f(s8_t *ifname, s8_t *mac)
{
	int sockfd = -1;
	struct ifreq ifr;
	struct sockaddr_in *addr = NULL;

	if (ifname == NULL)
		return ERROR_T;
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	addr->sin_family = AF_INET;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
		return ERROR_T;
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifr)) {
		close(sockfd);
		return ERROR_T;
	}
	memcpy(mac, ifr.ifr_hwaddr.sa_data, 6);
	close(sockfd);
	return OK_T;
}

status_t ifether_status_f(s8_t *ifname)
{
	int				fd;
	struct ifreq	ifr;
	struct mii_ioctl_data*	mii;

	if (ifname == NULL)
		return ERROR_T;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd <= 0)
		return ERROR_T;
	bzero(&ifr, sizeof(ifr));
	
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';
	if (ioctl(fd, SIOCGMIIPHY, &ifr) < 0) {
		close(fd);
		return ERROR_T;	
	}
	mii = (struct mii_ioctl_data*)&ifr.ifr_data;
	mii->reg_num = 0x01;
	if (ioctl(fd, SIOCGMIIREG, &ifr) < 0) {
		close(fd);
		return ERROR_T;
	}
	close(fd);
	if (mii->val_out & 0x0004) 
		return OK_T;
	else 
		return ERROR_T;
}


status_t sysRouteSet(s8_t *ifname, s32_t dest, s32_t mask, s32_t gate)
{
	return OK_T;
}

status_t ifether_get_f(s8_t *ifname, s32_t *ip, s32_t *mask)
{
	int sockfd = -1;
	struct ifreq ifr;
	struct sockaddr_in *addr = NULL;

	if (ifname == NULL)
		return ERROR_T;
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, ifname);
	addr = (struct sockaddr_in *)&ifr.ifr_addr;
	addr->sin_family = AF_INET;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_IP)) < 0)
		return ERROR_T;
	if (ioctl(sockfd, SIOCGIFADDR, &ifr)) {
		close(sockfd);
		return ERROR_T;
	}
	*ip = htonl(addr->sin_addr.s_addr);
	if (ioctl(sockfd, SIOCGIFNETMASK, &ifr)) {
		close(sockfd);
		return ERROR_T;
	}
	*mask = htonl(addr->sin_addr.s_addr);
	close(sockfd);
	return OK_T;
}





