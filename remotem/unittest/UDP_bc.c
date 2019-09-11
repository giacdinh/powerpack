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
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include "../../inc/common.h"

extern char * unit_ID();
extern int broadcast_id(char *);


int main()
{
    char unitid[32], *punitID;

	punitID = (char *) &unitid[0];	
    strcpy(punitID,(char *) unit_ID());

    while(1) 
	{
		printf("UDP broadcast from: %s\n", punitID);
        broadcast_id(punitID);
        sleep(BARK_DURATION);
    }
	return 0;
}

