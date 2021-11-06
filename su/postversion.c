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
#include "sys_msg.h"
#include "dev_config.h"

#define CLOUD_ACCEPT	"Accept: application/json"
#define POST_FW_INFO	"uid=%s&fwv=%s"
#define FW_INFO_URL		"%s/fw_info.php?"
#define NO_UPDATE		"No_update"

extern int get_su_file(char *);

/* Set test Gobal value. At final release this s */
static int Post_FW_Response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	int result = -1;
	logging(DBG_INFO,"%s: %s\n",__FUNCTION__,(char *) ptr);

	if(0 == strncmp(NO_UPDATE, (char *) ptr, 9))
	{
		logging(1,"%s: No update available\n",__FUNCTION__);
		sleep(2); // Make sure downstream dump is complete
		return nmemb;
	}

	result = get_su_file((char *) ptr);
	if(result == 0)
	{
		system("extractfw.sh");
		sleep(2);
		system("/mnt/sysdata/data/pp_fw_install.sh > /home/bacson/up.txt");
	}
	else
		logging(1,"Failed to download file\n");
    return nmemb;
}

static size_t Post_FW_Header_Response(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if(strstr(buffer, "HTTP"))
    {
		if(strstr(buffer, "200 OK"))
			logging(DBG_INFO,"%s: response OK\n", __FUNCTION__);	
    }

    return nitems;
}

int post_fw_info()
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
	char datafield[128], *pdata;
	char url[128], *pURL;
	static char uid[32], *puid = NULL;
	char coretemp[16];
	char version[16];
	char configURL[512];

	pdata = (char *) &datafield[0];
	pURL = (char *) &url[0];

	if(puid == NULL) // Init code, get unit Id and store
	{
		puid = (char *) &uid[0];
		strncpy(puid, (char *) unit_ID(),32);
	}
	
	get_str_json(POSTURL,(char *) &configURL[0]);
	get_version(&version[0]);
	sprintf(pdata,POST_FW_INFO,puid,&version[0]);
	sprintf(pURL,FW_INFO_URL,(char *) &configURL[0]);

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);
 
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Post_FW_Response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Post_FW_Header_Response);
		curl_easy_setopt(curl, CURLOPT_URL, pURL);
 
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
