#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <pthread.h>

#ifdef UNIT_DEBUG
#define DBG_SU  1
#else
#define DBG_SU  10
#endif

void *su_dog_bark_task();
void su_msg_handler(SU_MSG_T *Incoming);

void *su_main_task()
{
	int result = -1;
	pthread_t su_wd_id = -1;

    logging(DBG_INFO, "%s: Entering ...\n", __FUNCTION__);

    SU_MSG_T su_msg;
	logging(DBG_INFO, "Create controller message\n");
	int msgid = create_msg(SU_MSGQ_KEY);
	
	if(msgid < 0)
	{
		logging(DBG_ERROR,"Failed to create controller message queue\n");
		return 0;
	}	

	/* Create thread for SU WD response */
	if(su_wd_id == -1)
	{
		result = pthread_create(&su_wd_id, NULL, su_dog_bark_task, NULL);
        if(result == 0)
	        logging(DBG_DETAILED, "Starting su dog bark thread.\n");
        else
            logging(DBG_ERROR, "SU WD thread launch failed\n");
	}

	while(1) {
        recv_msg(msgid, (void *) &su_msg, sizeof(SU_MSG_T), MSG_TIMEOUT);
        su_msg_handler((SU_MSG_T *) &su_msg);
        usleep(100000);
	}

	return (void *) 0;
}

void *su_dog_bark_task()
{
    while(1) {
        send_dog_bark(SU_MODULE_ID);
        sleep(1);
    }
}

void su_msg_handler(SU_MSG_T *Incoming)
{
	SU_MSG_T *ctrl_msg;
    logging(DBG_INFO, "**** Incoming->modname: %s ****\n", modname[Incoming->header.moduleID]);
	
    switch(Incoming->header.subType)
    {
        default:
            logging(DBG_INFO, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->header.subType);
            break;
    }
}

