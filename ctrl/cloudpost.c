#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "common.h"
#include "ctrl_common.h"

#define CLOUD_ACCEPT	"Accept: application/json"
#define MAIN_URL		"http://bacson.tech/endpoint.php/postdata?"
#define POST_DATA		"uid=%s&coord=%s&temp=%s"

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
			logging(1,"%s\n",buffer);
    }
    return nitems;
}

int postdata(char *coordinate)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
	char datafield[128], *pdata;
	static char uid[32], *puid = NULL;
	
	pdata = (char *) &datafield[0];

	if(puid == NULL) // Init code, get unit Id and store
	{
		puid = (char *) &uid[0];
		strncpy(puid, (char *) unit_ID(),32);
	}
	
	sprintf(pdata,POST_DATA,puid,coordinate,get_core_temp());
    logging(1,"%s\n", pdata);

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);
 
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Cloud_Response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Cloud_Header_Response);

		curl_easy_setopt(curl, CURLOPT_URL, MAIN_URL);
 
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
//	curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, pdata);
        
        /* Now run off and do what you've been told! */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
		{
            printf("%s:curl_easy_perform() failed: %s\n",
			__FUNCTION__, curl_easy_strerror(res));
		}
    /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    curl_global_cleanup();

    return 1;
}

char *get_core_temp()
{
	char core_temp[16], *ret_temp;
	FILE *fd;
	if(access("/mnt/sysdata/log/core_temp", 0 ) == 0)
	{
		fd = fopen("/mnt/sysdata/log/core_temp", "r");
		if(fd > 0)
		{
			fscanf(fd, "%s",(char *) &core_temp[0]);
			ret_temp = (char *) &core_temp[0];
			fclose(fd);
			return ret_temp;
		}
	}	
	return "N/A";
}
