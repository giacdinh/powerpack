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
	static int time_set_init = -1, usb_init = -1, netw_issue = 0;  
	static int gps_cnt = 0, ping_cnt = 0;
	NMEA_RMC_T rmc;
	char coord[128];
	static int boot=1, power=0;
	int hat_pwr_status = -1, gps_ret = -1;
	logging(DBG_INFO,"%s: Entering ...\n", __FUNCTION__);

    while(1) 
	{

#ifndef USE_RASPI_HAT
		// Turn on USB hub
		logging(1,"Turn on USB\n");
		system("sudo echo '1-1' |sudo tee /sys/bus/usb/drivers/usb/bind");
#else
		hat_pwr_status = test_hat_power();
		//logging(1,"Device with GSM/GPS HAT is %d %s\n",hat_pwr_status, hat_pwr_status==0?"OFF":"ON");
		if(hat_pwr_status == 0)
		{
			logging(1,"Turn on HAT\n");
			system("sudo python /usr/local/bin/GSM_PWRKEY.py");
		}
		else if (hat_pwr_status == 1)
			logging(DBG_EVENT,"HAT port may be on, don't turn it on\n");
		else
			logging(DBG_ERROR,"May be something wrong with HAT\n");
		
#endif
		// Wait for 30 second for it to be ready to use
		sleep(10);

#ifndef USE_RASPI_HAT
#else
	init_raspi_hat_gps();
#endif

		// Clean up coordinate structure holder before each use
		bzero((void *) &rmc, sizeof(NMEA_RMC_T));

		//try to get time from gps and set system time
		gps_cnt++;
		if(gps_cnt > 5)
		{
			rmc.rlat = 0.000001;
			rmc.rlong = 0.000001;
			gps_cnt = 0;
			logging(DBG_ERROR,"%s: Can't get GPS use default coordinate\n", __FUNCTION__);
			goto use_default_gps;
		}

		gps_ret = get_gps_info(&rmc);
		if(gps_ret == GPS_NO_PORT)
		{
			logging(DBG_ERROR,"%s: Error return, can't get GPS\n", __FUNCTION__);
			sleep(5);
			continue;
		}
		else if(gps_ret == GPS_NO_DEV)
		{
			if(gps_cnt > 2) // try to get GPS device mount point couple time
			{
				logging(DBG_ERROR,"Force system reboot since no GPS device mount point\n");
				system("reboot");
			}
			sleep(5);
			continue;
		}
		else if(gps_ret == GPS_NO_SAT) // use default coordinate if no valid one available
		{
			rmc.rlat = 0.000001;
			rmc.rlong = 0.000001;
			gps_cnt = 0;
			logging(DBG_ERROR,"%s: Can't get GPS use default coordinate\n", __FUNCTION__);
			goto use_default_gps;
		}
		else
		{
			if(-1 == coord_validate(&rmc))
			{
				logging(DBG_ERROR, "Coordinate invalid. Sleep for a minute. GPScnt: %d\n", gps_cnt);
				sleep(60);
				continue;
			}
			else
				gps_cnt = 0;    // Reset when able to get GPS coordinate
		}

use_default_gps:
		// get core temperature 
		system("sudo /opt/vc/bin/vcgencmd measure_temp > /mnt/sysdata/log/core_temp");
		sleep(1);
		system("echo fwv=`/usr/local/bin/main_app -v |awk '{print $3}'` > /mnt/sysdata/log/version");
		// start cellular modem connection
		logging(DBG_EVENT,"Get cellular connection\n");

#ifndef USE_RASPI_HAT
		// Make sure PPP session start clean
		system("sudo hologram network disconnect");
		sleep(1);
		system("sudo hologram network connect");
		sleep(2);
#else
		logging(DBG_EVENT, "Setup PPP\n");
		system("sudo pppd call gprs-hologram &");
		sleep(30);
		// Setup up add route script and execute
		logging(DBG_EVENT, "Add route to network\n");
		system("sudo ip route flush 0/0");
		sleep(1);
		system("sudo route add default gw `ifconfig ppp0 |grep inet|cut -c 14-26` ppp0");
		//system("sudo chmod +x /tmp/addroute.sh");
		//system("sudo /tmp/addroute.sh");
		sleep(2);
#endif
		logging(DBG_EVENT,"Done cellular connection\n");

host_ping_trial:	
		//if(-1 == ping_host() && ping_cnt++ < 10)
		if(-1 == ping_host())
		{
			if(ping_cnt++ > 5)
			{
				ping_cnt = 0;
				logging(DBG_ERROR,"Can't connect to host. Skip this post\n");	
				netw_issue++;
				if(netw_issue > 2) // If device have 3 skip post reboot
					system("reboot");
				else
					netw_issue++;
				// disconnect modem and go back to waiting mode
#ifndef USE_RASPI_HAT
				system("sudo hologram network disconnect");	
#else
#endif
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
			netw_issue = 0; // reset network issue flag
			// Ready to post, check power source status
			power = get_power_source();
			bzero((void *) &coord[0], 128);
			sprintf((char *) &coord[0],"%f, %f", rmc.rlat, rmc.rlong);
			logging(DBG_EVENT, "coordinate: %s\n", (char *) &coord[0]);
			postdata((char *) &coord[0], boot, power);
			// Check and reset boot status flag
			if(boot == 1)
				boot = 0;
			
			// disconnect modem and go back to waiting mode
#ifndef USE_RASPI_HAT
			logging(DBG_EVENT,"Disconnect cellular\n");
			system("sudo hologram network disconnect");	
			sleep(2);
#endif
		}		
		logging(DBG_EVENT, "Sleep 4 hours after data report done\n");

#ifndef USE_RASPI_HAT
		// Turn of USB hub to conserv raspi energy in case of battery being used
		logging(1,"Turn off USB\n");
		system("sudo echo '1-1' |sudo tee /sys/bus/usb/drivers/usb/unbind");
#else
		logging(1,"Turn off HAT\n");
		system ("sudo killall pppd");
		//system ("sudo python /usr/local/bin/GSM_PWRKEY.py");
		sleep(5);
#endif
        sleep(REPORT_DELAY*60*60);		// Sleep for 4 hours
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
	logging(DBG_EVENT,"pinging host\n");
	hostinfo = gethostbyname(BACSON_HOST_NAME);
	if(hostinfo == NULL) // Not connected???
		return -1;

	return 1;
}

