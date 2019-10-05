#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include "sys_msg.h"
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <sys_msg.h>
#include "common.h"

#define ALLOWANCE_TIMER	30 // take action after 60 second
WD_RESPONSE_T modulelist[UNKNOWN_MODULE_ID];

void wd_msg_handler(GENERIC_MSG_HEADER_T *Incoming);
void UpdateBarkList(int Module);
void register_modules();
void wd_action();

char *modname[] = {"REMOTEM", "CTRL", "CONFIG"};
char *cmdname[] = {"Bark", "shutdown", "data"};

unsigned long get_sys_cur_time()
{
    struct timezone tz;
    struct timeval sysTime;

    gettimeofday(&sysTime, &tz);
    return sysTime.tv_sec;
}

void wdog_main_task()
{
    GENERIC_MSG_HEADER_T wd_msg;
    int msgid = create_msg(WD_MSGQ_KEY);
    if(msgid < 0)
    {
		logging(DBG_ERROR, "Failed to open WatchDog message\n");
		exit(0);
    }
    register_modules();

	// To prevent system reboot when system clock is not set properly
	sleep(BARK_DURATION*2);
    while(1) {
    	recv_msg(msgid, (void *) &wd_msg, sizeof(GENERIC_MSG_HEADER_T), 5);
    	wd_msg_handler((GENERIC_MSG_HEADER_T *) &wd_msg);
    	sleep(1);
    	wd_action();
    }
	exit(0);
}

void wd_msg_handler(GENERIC_MSG_HEADER_T *Incoming)
{
	logging(DBG_INFO, "*** Incoming->modname: %s ***\n", modname[Incoming->moduleID]);
    switch(Incoming->subType)
    {
		case MSG_BARK:
			UpdateBarkList(Incoming->moduleID);
			break;
		case MSG_REBOOT:
			system("reboot");
			break;
		default:
			logging(DBG_ERROR, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->subType);
			break;
    }
}

void UpdateBarkList(int Module)
{
    logging(DBG_INFO, "%s: ModID: %d Name: %s\n", __FUNCTION__, Module, modname[Module]);
    int i;
    for(i = 0; i < UNKNOWN_MODULE_ID; i++)
    {
    	if(modulelist[i].module_id == Module)
    	{
    		modulelist[i].timer = get_sys_cur_time();
			break;
    	}
    }
}

void register_modules()
{
    int i;
    for(i=0; i < UNKNOWN_MODULE_ID; i++)
    {
       	modulelist[i].module_id = i;
        modulelist[i].timer = get_sys_cur_time();
        modulelist[i].reboot = 0;
    }
}

void wd_action()
{
    int i;
    unsigned long lcur_time;
    lcur_time = get_sys_cur_time();
    for(i=0; i < UNKNOWN_MODULE_ID; i++)
    {
		// If not response in 60s warning
    	if( (lcur_time - modulelist[i].timer) > 60 && ((lcur_time - modulelist[i].timer) < 120) )
    		logging(DBG_ERROR, "Module: %s is marginally response...\n", modname[modulelist[i].module_id]);

		// If not response in 120s error 
    	else if( ((lcur_time - modulelist[i].timer) > 120) && ((lcur_time - modulelist[i].timer) < 180) )
    		logging(DBG_ERROR, "Module: %s is no longer response...\n", modname[modulelist[i].module_id]);

		else if( (lcur_time - modulelist[i].timer) > 180)
		{
	        logging(DBG_ERROR, "System about to be reboot\n");
			system("reboot");
		}
    }
}

void send_dog_bark(int from)
{
    GENERIC_MSG_HEADER_T bark;
    logging(DBG_INFO, " Rough ....rough from: %s id: %d\n", modname[from], from);
    bzero((char *) &bark, sizeof(GENERIC_MSG_HEADER_T));

    int msgid = open_msg(WD_MSGQ_KEY);
    if(msgid < 0)
    {
        logging(DBG_ERROR, "Error invalid message queue\n");
        return;
    }

    bark.subType = MSG_BARK;
    bark.moduleID = from;
    send_msg(msgid, (void *) &bark, sizeof(GENERIC_MSG_HEADER_T), 3);
}


void send_generic_msg(int from, int msg_id, int data)
{
    GENERIC_MSG_HEADER_T msg;
    bzero((char *) &msg, sizeof(GENERIC_MSG_HEADER_T));

    int msgid = open_msg(WD_MSGQ_KEY);
    if(msgid < 0)
    {
        logging(DBG_ERROR, "Error open message queue\n");
        return;
    }

    msg.subType = msg_id;
    msg.moduleID = from;
    msg.data = data;
    send_msg(msgid, (void *) &msg, sizeof(GENERIC_MSG_HEADER_T), 3);
}

void send_monitor_cmd(int from, char *cmd_str)
{
}
