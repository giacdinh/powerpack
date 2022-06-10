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
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "common.h"
#include "ctrl_common.h"
#include "sys_msg.h"

#ifdef UNIT_DEBUG
#define DBG_CTRL  1
#else
#define DBG_CTRL  10
#endif

void *ctrl_dog_bark_task();
void *ctrl_worker_task();
void ctrl_msg_handler(CTRL_MSG_T *Incoming);
int coord_validate(NMEA_RMC_T *rmc);
int ping_host();
int power_source_monitor();
extern void CTRL_send_SU_msg(int);

void *ctrl_main_task()
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

	(void) pthread_join(ctrl_wd_id, NULL);
	(void) pthread_join(ctrl_main_id, NULL);

	while(1) {
        recv_msg(msgid, (void *) &ctrl_msg, sizeof(CTRL_MSG_T), MSG_TIMEOUT);
        ctrl_msg_handler((CTRL_MSG_T *) &ctrl_msg);
        sleep(1);
	}
	return (void *) 0;
}

void *ctrl_dog_bark_task()
{
    while(1) {
        send_dog_bark(CTRL_MODULE_ID);
        sleep(BARK_DURATION);
    }
	return (void *) 0;
}

void *ctrl_worker_task()
{
	static int time_set_init = -1, usb_init = -1, netw_issue = 0, device_reboot = 0;  
	int simgps = 0, gpsstart, gpsend, ping_cnt, result;
	float lat,lng;
	char coord[128];
	static int boot=1, power=0;
	int hat_pwr_status = -1, gps_ret = -1;
	logging(DBG_INFO,"%s: Entering ...\n", __FUNCTION__);

    while(1) 
	{
		power = get_power_source();
		gpsstart = get_uptime();	
		simgps = get_gps_info(&lat, &lng);
		if(simgps == -1)
		{
			lat = 0.000001;
			lng = 0.000001;
		}
		gpsend = get_uptime();	
		logging(1, "GPS start: %lu end: %lu total: %lu\n", gpsstart, gpsend, (gpsend-gpsstart));
		// get core temperature 
		system("sudo /opt/vc/bin/vcgencmd measure_temp > /mnt/sysdata/log/core_temp");
		sleep(1);
		system("echo fwv=`/usr/local/bin/powerpack -v |awk '{print $3}'` > /mnt/sysdata/log/version");
		// start cellular modem connection
		logging(DBG_CTRL,"Get cellular connection\n");
		logging(DBG_CTRL, "Setup PPP. Network routing should be handled by PPP\n");
		logging(DBG_CTRL,"Use Hologram cellular connection\n");
		system("sudo pppd call gprs-hologram &");
		sleep(20);
		system("sudo echo `ifconfig ppp0 |grep inet` >> /mnt/sysdata/log/`date -I |awk -F '-' '{print $1$2$3}'`'_log'");
		logging(DBG_EVENT,"Done cellular connection\n");

host_ping_trial:	
		if(-1 == ping_host())
		{
			if(ping_cnt++ > PING_TIME)
			{
				ping_cnt = 0; 
				logging(DBG_ERROR,"Can't connect to host. Skip this post\n");	
				netw_issue++;
				if(netw_issue > 2) // If device have 3 skip post reboot
					system("reboot");
				else
					netw_issue++;
			}
			else
			{
				sleep(2);
				logging(DBG_EVENT, "Trying to ping host\n");
				goto host_ping_trial;
			}
		}
		else
		{
			// simgps=0 GPS=Hologram, simgps=1 GPS=true, simgps=2 GPS=SIM
			netw_issue = 0; // reset network issue flag
			// Ready to post, check power source status
			bzero((void *) &coord[0], 128);
			sprintf((char *) &coord[0],"%f, %f", lat, lng);
			logging(DBG_CTRL, "coordinate: %s\n", (char *) &coord[0]);
			int postresult = 0;
			postresult = postdata((char *) &coord[0], boot, power,(gpsend-gpsstart), simgps);
			if(postresult == -1)
			{
				sleep(30);
				logging(1, "Try to post one more time before give up\n");
				postresult = postdata((char *) &coord[0], boot, power, (gpsend-gpsstart), simgps);
			}
		}		

		if(power == 0) // Run from battery, will flag controller to shutdown power
		{
			logging(1,"System run on battery. Let controller shutdown to conserve power\n");
			sync();
			sleep(1);	// Ready after sync
			set_gpio_pin16_high();
			sleep(5);	// Power control management will shutdown the RASPI
			set_gpio_pin16_reset(); // Reset pin
			system ("sudo killall pppd");
			logging(DBG_ERROR,"Thing shouldn't be getting here\n");
			// If it getting here just do normal report
			sleep(REPORT_DELAY*60*60);		// Sleep for 6 hours
		}
		else	// If run on AC just sleep and wake up after REPORT DELAY timer 
		{
			if(boot == 1)	// Send message to SU to check for update and Reset 
			{
				boot = 0;
				logging(1,"Send SU message\n");
				CTRL_send_SU_msg(MSG_SU_CHECK);
				sleep(300); // wait 5 mins until sofware upgrade complete
			}
			logging(1,"kill HAT pppd session\n");
			system ("sudo killall pppd");

			logging(DBG_EVENT, "Sleep %d hours after data report done\n", REPORT_DELAY);
			sleep(REPORT_DELAY*60*60);		// Sleep for 6 hours
			device_reboot++;	
			// If system run for more than 48h, then reboot device
			// This make sure the system at healthy state all the time
			if((device_reboot*REPORT_DELAY) > PERIODIC_REBOOT)
			{
				logging(1,"System periodically reboot\n");
				system("sudo reboot");
			}
		}
		logging(DBG_EVENT, "Wakeup to report data\n");
    }
	return (void *) 0;
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
	struct hostent *hostinfo = NULL;
	char ip[16];
	struct sockaddr_in sock_addr;
	int i;

	hostinfo = gethostbyname(BACSON_HOST_NAME);
	if(hostinfo == NULL) // Not connected???
	{
		//If dedicate host not found try to double check ping popular host. Ex google.com
		logging(DBG_ERROR,"pinging host failed trying second popular host\n");
		hostinfo = gethostbyname(POPHOST1);
		if(hostinfo == NULL)
		{
			logging(DBG_ERROR,"pinging host failed\n");
			return -1;
		}
	}
    for (i = 0; hostinfo->h_addr_list[i]; ++i) {
        sock_addr.sin_addr = *((struct in_addr*) hostinfo->h_addr_list[i]);
        inet_ntop(AF_INET, &sock_addr.sin_addr, ip, sizeof(ip));
        logging(DBG_EVENT,"hostname: %s %s\n",BACSON_HOST_NAME, ip);
    }

	return 1;
}

void CTRL_send_SU_msg(int key)
{
    SU_MSG_T msg;
    logging(DBG_INFO, "%s: msg key %d\n", __FUNCTION__, key);
    bzero((char *) &msg, sizeof(SU_MSG_T));

    int msgid = open_msg(SU_MSGQ_KEY);
    if(msgid < 0)
    {
        logging(DBG_ERROR, "Error invalid message queue\n");
        return;
    }

    msg.header.subType = key;
    msg.header.moduleID = CTRL_MODULE_ID;
    send_msg(msgid, (void *) &msg, sizeof(SU_MSG_T), 3);
}

