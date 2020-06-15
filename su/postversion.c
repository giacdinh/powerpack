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
#define POST_DATA		"uid=%s&fwv=%s"

extern int get_version(char *);

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

	pdata = (char *) &datafield[0];

	if(puid == NULL) // Init code, get unit Id and store
	{
		puid = (char *) &uid[0];
		strncpy(puid, (char *) unit_ID(),32);
	}
	
	get_version(&version[0]);
	sprintf(pdata,POST_DATA,puid,&version[0]);
    logging(DBG_EVENT,"%s\n", pdata);

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);
 
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Cloud_Response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Cloud_Header_Response);
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
