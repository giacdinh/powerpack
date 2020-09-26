/*************************************************************************
 * 
 * Bac Son Technologies  
 * __________________
 * 
 *  [2019] Bac Son Technologies LLC 
 *  All Rights Reserved.
 * 
 * NOTICE:  All information contained herein is, and remains
 * the property of Bac Son Technologies LLC and its suppliers,
 * if any.  The intellectual and technical concepts contained
 * herein are proprietary to Bac Son Technologies LLC 
 * and its suppliers and may be covered by U.S. and Foreign Patents,
 * patents in process, and are protected by trade secret or copyright law.
 * Dissemination of this information or reproduction of this material
 * is strictly forbidden unless prior written permission is obtained
 * from Bac Son Technologies LLC.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"

#define BROADCAST_PORT	1999	 // change to 1999 cause 2999 is remote control

void broadcast_id(char *mess, unsigned char *ip)
{
	int sock;                        
	struct sockaddr_in broadcastAddr; 
	unsigned short broadcastPort;     
	char *sendString;                 
	int broadcastPermission;         
	int sendStringLen;                

	broadcastPort = BROADCAST_PORT;
	sendString = mess;             /*  string to broadcast */

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0){
       fprintf(stderr, "socket error");
        exit(1);
	}

	broadcastPermission = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (void *) &broadcastPermission,
			sizeof(broadcastPermission)) < 0)
	{
		fprintf(stderr, "setsockopt error");
		exit(1);
	}
	char loopch=0;

	if (setsockopt(sock, IPPROTO_IP, IP_MULTICAST_LOOP,
               (char *)&loopch, sizeof(loopch)) < 0) 
	{
		perror("setting IP_MULTICAST_LOOP:");
		close(sock);
		exit(1);
	}

	/* Construct local address structure */
	memset(&broadcastAddr, 0, sizeof(broadcastAddr));   
	broadcastAddr.sin_family = AF_INET;                 
	broadcastAddr.sin_addr.s_addr = inet_addr(ip);
	broadcastAddr.sin_port = htons(broadcastPort);       

	sendStringLen = strlen(sendString);  

    /* Broadcast sendString in datagram to clients */
    if (sendto(sock, sendString, sendStringLen, 0, (struct sockaddr *)&broadcastAddr, 
			sizeof(broadcastAddr)) != sendStringLen)
	{
            fprintf(stderr, "send to error: %d", errno);
            exit(1);
	}
	close(sock);
}

