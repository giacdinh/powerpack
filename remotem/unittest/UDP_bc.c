#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <net/if.h>
#include "../../inc/common.h"

extern char * unit_ID();
extern int broadcast_id(char *, unsigned char*);
int getIP_string(unsigned char *ownIP);

int main()
{
    char unitid[32], *punitID;
	unsigned char ip[16];
	punitID = (char *) &unitid[0];	
    strcpy(punitID,(char *) unit_ID());

	getIP_string(&ip[0]);
    while(1) 
	{
		printf("UDP broadcast from: %s ip: %s\n", punitID, &ip[0]);
        broadcast_id(punitID, &ip[0]);
        sleep(BARK_DURATION);
    }
	return 0;
}

int getIP_string(unsigned char *ownIP)
{
    unsigned char ip_address[15];
    int fd;
    struct ifreq ifr;
    
    /*AF_INET - to define network interface IPv4*/
    /*Creating soket for it.*/
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    /*AF_INET - to define IPv4 Address type.*/
    ifr.ifr_addr.sa_family = AF_INET;

    /*eth0 - define the ifr_name - port name
    where network attached.*/
    //memcpy(ifr.ifr_name, "wlp6s0", IFNAMSIZ-1);
    memcpy(ifr.ifr_name, UDP_BC_DEV, IFNAMSIZ-1);
    
    /*Accessing network interface information by
    passing address using ioctl.*/
    ioctl(fd, SIOCGIFADDR, &ifr);
    /*closing fd*/
    close(fd);
    
    /*Extract IP Address*/
    strcpy(ip_address,inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	int i,octetcnt = 0;
	for(i = 0; i < 15; i++)
	{
		if(ip_address[i] == '.')
			octetcnt++; 

		if(octetcnt == 3)
		{
			ip_address[++i] = '2';
			ip_address[++i] = '5';
			ip_address[++i] = '5';
		}
	}
//    printf("Broadcast IP Address is: %s\n",ip_address);
	strncpy(ownIP, &ip_address[0], 15);
    return 0;
} 
