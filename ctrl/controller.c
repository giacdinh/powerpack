#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include "common.h"
#include "ctrl_common.h"

void *ctrl_dog_bark_task();
void *ctrl_worker_task();
void ctrl_msg_handler(CTRL_MSG_T *Incoming);
int coord_validate(NMEA_RMC_T *rmc);
int ping_host();

int ctrl_main_task()
{
	int result = -1;
	pthread_t ctrl_wd_id = -1, ctrl_main_id = -1;

    CTRL_MSG_T ctrl_msg;
	logging(DBG_INFO, "Create controller message\n");
	int msgid = create_msg(CTRL_MSGQ_KEY);
	
	if(msgid < 0)
	{
		logging(DBG_ERROR,"Failed to create controller message queue\n");
		return 0;
	}	

	/* Create thread for WD response */
	if(ctrl_wd_id == -1)
	{
		result = pthread_create(&ctrl_wd_id, NULL, ctrl_dog_bark_task, NULL);
        if(result == 0)
	        logging(DBG_DETAILED, "Starting controller dog bark thread.\n");
        else
            logging(DBG_ERROR, "CTRL WD thread launch failed\n");
	}

	/* Create thread for CTRL main task */
	if(ctrl_main_id == -1)
	{
		result = pthread_create(&ctrl_main_id, NULL, ctrl_worker_task, NULL);
        if(result == 0)
	        logging(DBG_DETAILED, "Starting controller main task thread.\n");
        else
            logging(DBG_ERROR, "CTRL main task thread launch failed\n");
	}

	while(1) {
        recv_msg(msgid, (void *) &ctrl_msg, sizeof(CTRL_MSG_T), MSG_TIMEOUT);
        ctrl_msg_handler((CTRL_MSG_T *) &ctrl_msg);
        usleep(10000);
	}
}

void *ctrl_dog_bark_task()
{
    while(1) {
        send_dog_bark(CTRL_MODULE_ID);
        sleep(1);
    }
}

void *ctrl_worker_task()
{
	static int time_set_init = -1;  
	static int gps_cnt = 0;
	NMEA_RMC_T rmc;
	char coord[128];
	logging(1,"%s: Entering ...\n", __FUNCTION__);
    while(1) 
	{
		//try to get time from gps and set system time
		if(!get_gps_info(&rmc))
		{
			if(gps_cnt++ > 5)
			{
				rmc.rlat = 0.000001;
				rmc.rlat = 0.000001;
				gps_cnt = 0;
				logging(DBG_ERROR,"%s: Can't get GPS use default coordinate\n", __FUNCTION__);
				goto use_default_gps;
			}
			logging(DBG_ERROR,"%s: Error return\n", __FUNCTION__);
			sleep(5);
			continue;
		}
		else
			gps_cnt = 0;	// Reset when able to get GPS coordinate
		
		if(-1 == coord_validate(&rmc))
		{
			logging(1, "Coordinate invalid. Sleep for a minute then loop for more data\n");
			sleep(5);
			continue;
		}

use_default_gps:
		// get core temperature 
		system("sudo /opt/vc/bin/vcgencmd measure_temp > /tmp/temperature");
		sleep(1);
		// start cellular modem connection
		system("sudo hologram network connect");
		sleep(2);
	
		if(-1 == ping_host())
		{
			logging(DBG_ERROR,"Can't connect to host. Skip this post\n");	
			// disconnect modem and go back to waiting mode
			system("sudo hologram network disconnect");	
			sleep(2);
		}
		else
		{
			sprintf((char *) &coord[0],"%f, %f", rmc.rlat, rmc.rlong);
			logging(1, "coordinate: %s\n", (char *) &coord[0]);
			postdata((char *) &coord[0]);
			
			// disconnect modem and go back to waiting mode
			system("sudo hologram network disconnect");	
			sleep(2);
		}		
        sleep(6*60*60);
    }
}

void ctrl_msg_handler(CTRL_MSG_T *Incoming)
{
	CTRL_MSG_T *ctrl_msg;
    logging(DBG_INFO, "**** Incoming->modname: %s ****\n", modname[Incoming->header.moduleID]);
	
    switch(Incoming->header.subType)
    {
        default:
            logging(DBG_INFO, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->header.subType);
            break;
    }
}

int coord_validate(NMEA_RMC_T *rmc)
{
	if(rmc->rlat == 0.0 || rmc->rlong == 0.0)
	{
		logging(DBG_ERROR, "Invalid coordinate. lat: %f long:%f\n", rmc->rlat, rmc->rlong);
		return -1;
	}
	return 1;
}

int ping_host()
{
	struct hostent *hostinfo;
	hostinfo = gethostbyname(BACSON_HOST_NAME);
	if(hostinfo == NULL) // Not connected???
		return -1;

	return 1;
}


