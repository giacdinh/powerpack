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
int post_fw_info();

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
		case MSG_SU_CHECK:
			logging(1, "%s: Time to check for Software update\n",__FUNCTION__);
			post_fw_info();
		break;
        default:
            logging(DBG_INFO, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->header.subType);
            break;
    }
}

