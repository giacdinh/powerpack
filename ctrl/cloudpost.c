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
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "ctrl_common.h"
#include "sys_msg.h"
#include "dev_config.h"

#define CLOUD_ACCEPT	"Accept: application/json"
#define DEFAULT_URL		"http://bacsontech.com/postdata.php/postdata?"
#define POST_DATA		"uid=%s&coord=%s&fwv=%s&temp=%s&boot=%d&pwr=%d"

static int server_connect = 1; //use this flag to toggle between default and config URL

int get_config_url(char *url);

/* Set test Gobal value. At final release this s */
static int Cloud_Response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	logging(1,"%s\n", (char *) ptr);
    return nmemb;
}

static size_t Cloud_Header_Response(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if(strstr(buffer, "HTTP"))
    {
		if(strstr(buffer, "200 OK"))
			logging(1,"response OK\n");	
		else
		{
			server_connect = 0;
			logging(DBG_INFO,"%s\n",buffer);
		}
    }

    return nitems;
}

int postdata(char *coordinate, int boot, int power)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
	char datafield[128], *pdata;
	static char uid[32], *puid = NULL;
	char coretemp[16];
	char version[16];
	char configURL[512];

	if(server_connect == 1)
	{
		bzero((char *) &configURL[0], 512);
		get_config_url(&configURL[0]);
		logging(1, "Config URL: %s\n", (char *) &configURL[0]);
	}
	
	pdata = (char *) &datafield[0];

	if(puid == NULL) // Init code, get unit Id and store
	{
		puid = (char *) &uid[0];
		strncpy(puid, (char *) unit_ID(),32);
	}
	
	// get core temperature
	get_core_temp(&coretemp[0]);
	get_version(&version[0]);
	sprintf(pdata,POST_DATA,puid,coordinate,&version[0],&coretemp[0],boot,power);
    logging(DBG_EVENT,"%s\n", pdata);

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);
 
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Cloud_Response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Cloud_Header_Response);

		if(server_connect == 0) // If config URL failed use default URL
			curl_easy_setopt(curl, CURLOPT_URL, DEFAULT_URL);
		else if(server_connect == 1)
			curl_easy_setopt(curl, CURLOPT_URL,(char *) &configURL[0]);
 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata);
        
        /* Now run off and do what you've been told! */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
		{
            logging(DBG_ERROR,"%s:curl_easy_perform() failed: %s\n",
			__FUNCTION__, curl_easy_strerror(res));
        
			curl_easy_setopt(curl, CURLOPT_URL, DEFAULT_URL);
			if(res != CURLE_OK)
				logging(DBG_ERROR,"%s: Repost use DEFAULT_URL curl_easy_perform() failed: %s\n",
					__FUNCTION__, curl_easy_strerror(res));
			
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return -1;
		}
    /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return 1;
}

int get_core_temp(char *coretemp)
{
	char core_temp[16], *ret_temp;
    FILE *fd;
    if(access("/mnt/sysdata/log/core_temp", 0 ) == 0)
    {
        fd = fopen("/mnt/sysdata/log/core_temp", "r");
        if(fd > 0)
        {
            fscanf(fd, "temp=%s",(char *) &core_temp[0]);
            fclose(fd);
            strncpy(coretemp, (char *) &core_temp[0], 16);
            return 1;
        }
    }
    strcpy(coretemp,"N/A");
    return -1;
}

int get_version(char *version)
{
	char dversion[16];
	FILE *fd;
	if(access("/mnt/sysdata/log/version", 0 ) == 0)
	{
        fd = fopen("/mnt/sysdata/log/version", "r");
        if(fd > 0)
        {
            fscanf(fd, "fwv=%s",(char *) &dversion[0]);
            fclose(fd);
            strncpy(version, (char *) &dversion[0], 16);
            return 1;
        }
	}
	strcpy(version,"N/A");
	return -1;
}

int get_config_url(char *url)
{
	char posturl[256], endpoint[256];
	get_str_json(POSTURL,(char *) &posturl[0]);
	get_str_json(ENDPOINT,(char *) &endpoint[0]);
	sprintf(url,"%s%s",&posturl[0],&endpoint[0]);

	logging(DBG_DBG,"%s: URL: %s\n", __FUNCTION__, url);
	return 0;
}
