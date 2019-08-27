#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
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

#include <common.h>
#include "remotem.h"
#include "remotem_common.h"
#include <sys_msg.h>

#define REMOTEM_LOCAL_IPADDR    "127.0.0.1"
static struct sockaddr_in gWebServerAddr;
#define REMOTEM_WAIT_PORT_AVAILABLE (60)  //seconds
#define REMOTEM_BACKLOG         (10)
#define REQUEST_MAX				(256)

#ifdef UNIT_DEBUG
#define DBG_REM  1
#else
#define DBG_REM  10
#endif

void remotem_msg_handler(REMOTEM_MSG_T *Incoming);
void *remotem_dog_bark_task();
void *remotem_com_task();
void REMOTEMActionSIGCHLD(int sigNum, siginfo_t *pInfo, void *unused);
int remotem_worker_process();
void remotem_tag_read();
int remotem_request_handler(int);
void send_msg_to_reader(int msg_id);
int remotem_socket_create(void);
void request_response_header(int new_remotem_sock);
char *unit_ID();

void *remotem_main_task()
{
    REMOTEM_MSG_T remotem_msg;
	logging(DBG_DBG, "Create remotem message\n");
    int msgid = create_msg(REMOTEM_MSGQ_KEY);
	int result = -1;

	pthread_t remotem_dog_id = -1, remotem_com_id = -1;
    if(msgid < 0)
    {
        printf("Failed to create remotem message queue\n");
        return 0;
    }

	/* Create thread for watchdog response */
	if(remotem_dog_id == -1)
	{
		result = pthread_create(&remotem_dog_id, NULL, remotem_dog_bark_task, NULL);
		if(result == 0)
			logging(DBG_INFO, "Starting remotem dog bark thread.\n");
		else
			logging(DBG_ERROR, "REMOTEM WD response thread launch failed\n");
	}
	
	/* Create connection handler */
	if(remotem_com_id == -1)
	{
		result = pthread_create(&remotem_com_id, NULL, remotem_com_task, NULL);
		if(result == 0)
			logging(DBG_INFO, "Starting remotem com thread.\n");
		else
			logging(DBG_ERROR, "REMOTEM COM thread launch failed\n");
	}

	/* join both thread before run */
	
    (void) pthread_join(remotem_dog_id, NULL);
    (void) pthread_join(remotem_com_id, NULL);

	while(1) {
		logging(DBG_DBG, " REMOTEM Main task loop\n");
        recv_msg(msgid, (void *) &remotem_msg, sizeof(REMOTEM_MSG_T), MSG_TIMEOUT);
        remotem_msg_handler((REMOTEM_MSG_T *) &remotem_msg);
        sleep(1); //Should be lesser sleep
	}
	return (void *) 0;
}

void remotem_msg_handler(REMOTEM_MSG_T *Incoming)
{
    logging(DBG_INFO, "**** Incoming->modname: %s ****\n", modname[Incoming->header.moduleID]);
    switch(Incoming->header.subType)
    {
        default:
            logging(DBG_INFO, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->header.subType);
            break;
    }
}

void *remotem_dog_bark_task()
{
    char unitid[32], *punitID;

	punitID = (char *) &unitid[0];	
    strcpy(punitID,(char *) unit_ID());

    while(1) 
	{
		send_dog_bark(REMOTEM_MODULE_ID);
        broadcast_id(punitID);
        sleep(BARK_DURATION);
    }
	return (void *) 0;
}

void *remotem_com_task()
{
    struct sockaddr_in remotem_socket_addr;
    int remotem_socketId = -1, new_remotem_sock = -1;
    int sockaddr_size;

    remotem_socketId = remotem_socket_create();
    if(remotem_socketId < 0)
    {
        logging(DBG_ERROR, "%s %d: Failed to create socket \n", __FUNCTION__, __LINE__);
    }

    logging(DBG_DBG, "%s %d: looping for remote connect sock: %d\n", __FUNCTION__, __LINE__, remotem_socketId);
    if((listen(remotem_socketId, REMOTEM_BACKLOG)) < 0)
    {
        if (errno != EINTR)
        {
            logging(DBG_ERROR, "%s %d: Socket: %d failed to listen\n", __FUNCTION__, __LINE__, remotem_socketId);
            close(remotem_socketId);
            exit(0);
        }
    }
    logging(DBG_INFO, "%s %d: Socket: %d listened\n", __FUNCTION__, __LINE__, remotem_socketId);

    if((listen(remotem_socketId, REMOTEM_BACKLOG)) < 0)
    {
        if (errno != EINTR)
        {
            logging(DBG_ERROR, "%s %d: Socket: %d failed to listen\n", __FUNCTION__, __LINE__, remotem_socketId);
            close(remotem_socketId);
            exit(0);
        }
    }
    logging(DBG_INFO, "%s %d: Socket: %d listened\n", __FUNCTION__, __LINE__, remotem_socketId);

    /* Accepting socket */
    sockaddr_size = sizeof(struct sockaddr_in);
    memset(&remotem_socket_addr, 0, sockaddr_size);

    while (new_remotem_sock = accept(remotem_socketId,(struct sockaddr *) &remotem_socket_addr, &sockaddr_size))
	{
		remotem_request_handler(new_remotem_sock);
	}
}

int remotem_socket_create(void)
{
    struct sockaddr_in REMOTEMSocketAddr;
    int bindTmout = REMOTEM_WAIT_PORT_AVAILABLE;
    int sock = -1; 
    int rc = -1;
    struct linger lng;
    int reuseAddr = 1;

	logging(DBG_INFO, "%s %d: Entering socket create...\n", __FUNCTION__, __LINE__);
    /* Create a Socket using the Security Manager API .*/
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == sock)
    {
		logging(DBG_ERROR, "%s %d: Failed to create socket\n", __FUNCTION__, __LINE__);
        return -1;
    }
    memset(&REMOTEMSocketAddr, 0, sizeof(REMOTEMSocketAddr));
    REMOTEMSocketAddr.sin_family = AF_INET;
    REMOTEMSocketAddr.sin_port = htons(REMOTE_CGI_PORT);
    REMOTEMSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    lng.l_onoff  = 0;
    lng.l_linger = 1;

    setsockopt(sock, SOL_SOCKET,SO_LINGER, (const void *)&lng, sizeof(lng));
    setsockopt(sock, SOL_SOCKET,SO_REUSEADDR, (const void *)&reuseAddr, sizeof(reuseAddr));

    while (bindTmout > 0)
    {
        if ((rc = bind(sock, (struct sockaddr *) &REMOTEMSocketAddr, sizeof(REMOTEMSocketAddr))) >= 0)
        {
            bindTmout = 0;
        }
        else
        {
			logging(DBG_ERROR, "%s %d: bin() Failed err=%s(%d)\n", 
				__FUNCTION__, __LINE__, strerror(errno), errno);
            bindTmout--;
        }
    }

    if (rc < 0)
    {
			logging(DBG_ERROR, "%s %d: bin() Failed err=%s(%d)\n", 
				__FUNCTION__, __LINE__, strerror(errno), errno);
        return -1;
    }

    return sock;
}

int remotem_request_handler(int new_remotem_sock)
{
	char income_request[REQUEST_MAX];
	int read_byte;

	read_byte = read(new_remotem_sock, &income_request, REQUEST_MAX);
	logging(1, "Request: %s\n", &income_request);
	request_response_header(new_remotem_sock);
	if(NULL != strstr((char *) &income_request[0], REMOTEM_TEST_CMD))
	{
		logging(1,"REMOTEM TEST CMD\n");	
	}
	if(NULL != strstr((char *) &income_request[0], REMOTEM_DEV_ID))
	{
		logging(1,"REMOTEM Device ID\n");
		write(new_remotem_sock, GENERAL_RESPONSE_HEADER, strlen(GENERAL_RESPONSE_HEADER));
        write(new_remotem_sock, unit_ID(), 8);
	}
    else if(NULL != strstr((char *) &income_request[0], REMOTEM_SYS_SHUTDOWN))
    {
        logging(1,"System shutdown\n");
        system("sudo shutdown -h now");
    }
    else if(NULL != strstr((char *) &income_request[0], REMOTEM_SYS_REBOOT))
    {
        logging(1,"System reboot\n");
        system("sudo reboot");
    }
	write(new_remotem_sock, "200 OK\n", 7);
	close(new_remotem_sock);
	return 0;
}
	

void request_response_header(int new_remotem_sock)
{
	write(new_remotem_sock, GENERAL_RESPONSE_HEADER, strlen(GENERAL_RESPONSE_HEADER));
}
