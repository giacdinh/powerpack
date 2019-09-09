#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "dev_config.h"
#include "common.h"

void *config_dog_bark_task();
void config_msg_handler(CONFIG_MSG_T *Incoming);
void *config_cmd_task();
const char *process_json_data(char *ptr, char *search_key, int  *getint);
int config_init();

#ifdef UNIT_DEBUG
#define DBG_CONFIG	1
#else
#define DBG_CONFIG  10
#endif

void *conf_main_task()
{
	int result = -1;
	pthread_t config_wd_id = -1, config_cmd_id = -1;
	CTRL_MSG_T config_msg;

    logging(DBG_INFO, "%s: Entering ...\n", __FUNCTION__);

	logging(DBG_INFO, "Create controller message\n");
	int msgid = create_msg(CONFIG_MSGQ_KEY);
	
	if(msgid < 0)
	{
		logging(DBG_ERROR,"Failed to create config message queue\n");
		return 0;
	}	

	/* Create thread for CONFIG WD response */
	if(config_wd_id == -1)
	{
		result = pthread_create(&config_wd_id, NULL, config_dog_bark_task, NULL);
        if(result == 0)
	        logging(DBG_DETAILED, "Starting config dog bark thread.\n");
        else
            logging(DBG_ERROR, "CONFIG WD thread launch failed\n");
	}

	/* Create thread for CONFIG CMD */
	if(config_cmd_id == -1)
	{
		result = pthread_create(&config_cmd_id, NULL, config_cmd_task, NULL);
        if(result == 0)
	        logging(DBG_DETAILED, "Starting config cmd thread.\n");
        else
            logging(DBG_ERROR, "CONFIG CMD thread launch failed\n");
	}

	logging(DBG_CONFIG,"%s %d: Config data init ...\n", __FUNCTION__, __LINE__);
	config_init();
    (void) pthread_join(config_wd_id, NULL);
    (void) pthread_join(config_cmd_id, NULL);

	while(1) {
        recv_msg(msgid, (void *) &config_msg, sizeof(CONFIG_MSG_T), MSG_TIMEOUT);
        config_msg_handler((CONFIG_MSG_T *) &config_msg);
        sleep(1);
	}
	
	return (void *) 0;
}

void *config_dog_bark_task()
{
	// config start well ahead of most module bark need to wait
	logging(1,"%s: Entering\n", __FUNCTION__);
	sleep(7);
    while(1) {
        send_dog_bark(CONFIG_MODULE_ID);
        sleep(BARK_DURATION);
    }
}

void *config_cmd_task()
{
    logging(DBG_INFO, "%s: Entering ...\n", __FUNCTION__);
    sleep(5);

    while(1)
    {
	
		sleep(1); // Do monitor trigger every quarter of second
    }
}

void config_msg_handler(CONFIG_MSG_T *Incoming)
{
	CONFIG_MSG_T *config_msg;
    logging(DBG_INFO, "**** Incoming->modname: %s ****\n", modname[Incoming->header.moduleID]);
	
    switch(Incoming->header.subType)
    {
        default:
            logging(DBG_INFO, "%s: Unknown message type %d\n",__FUNCTION__, Incoming->header.subType);
            break;
    }
}

int config_init()
{
	int fp, rbyte, i;
	char confbuf[CONFIG_FILE_SZ];
	CONFIG_T config;

	logging(DBG_CONFIG,"%s %d: Entering ...\n", __FUNCTION__, __LINE__);
	
	fp = open(CONFIG_FILE, O_RDONLY);
	if(fp < 0)
	{
		logging(DBG_ERROR, "Open config.json failed\n");
		return -1;
	}

	rbyte = read(fp, &confbuf[0], CONFIG_FILE_SZ);

	if(rbyte > 0)
	{
		for(i=0; i < CONFIG_ENUM_UNKNOWN; i++)
		{
			switch(i) {
				case UNITID:
					strncpy(config.unitid, process_json_data((char *) &confbuf[0], CONFIG_FIELD[UNITID], 0),MAX_CONFIG_SZ);
                    logging(DBG_CONFIG, "UNITID: %s\n", config.unitid);
					break;
				case POSTURL:
					strncpy(config.posturl, process_json_data((char *) &confbuf[0], CONFIG_FIELD[POSTURL], 0),MAX_CONFIG_SZ);
                    logging(DBG_CONFIG, "posturl: %s\n", config.posturl);
					break;
				case ENDPOINT:
					strncpy(config.endpoint, process_json_data((char *) &confbuf[0], CONFIG_FIELD[ENDPOINT], 0),MAX_CONFIG_SZ);
                    logging(DBG_CONFIG, "Endpoint: %s\n", config.endpoint);
					break;
				case HOMELAT:
					strncpy(config.homelat, process_json_data((char *) &confbuf[0], CONFIG_FIELD[HOMELAT], 0),MAX_CONFIG_SZ);
                    logging(DBG_CONFIG, "Home Lat: %s\n", config.homelat);
					break;
				case HOMELONG:
					strncpy(config.homelong, process_json_data((char *) &confbuf[0], CONFIG_FIELD[HOMELONG], 0),MAX_CONFIG_SZ);
                    logging(DBG_CONFIG, "Home Long: %s\n", config.homelong);
					break;
				case METRIC:
					process_json_data((char *) &confbuf[0], CONFIG_FIELD[METRIC], &(config.metric));
					logging(DBG_CONFIG,"Metric: %s\n", config.metric==1?"F":"C");
					break;
				default:
					logging(DBG_ERROR, "CONFIG FIELDs loading ID invalid: %d\n", i);
					break;
			}

		}
	}	
	else
		logging(DBG_ERROR,"Attempt to read config file failed\n");

	return 0;	
}

