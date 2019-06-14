#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>

#define CLOUD_ACCEPT        "Accept: application/json"
#define MAIN_URL			"http://bacson.tech/endpoint.php/postdata?"
#define POST_DATA "uid=pp_000000001&coord=28.703239,%20-81.204086"

/* Set test Gobal value. At final release this s */
static int Cloud_Response(void *ptr, size_t size, size_t nmemb, void *stream)
{
	printf("%s: %s\n", __FUNCTION__, ptr);
    return nmemb;
}

static size_t Cloud_Header_Response(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if(strstr(buffer, "HTTP"))
    {
		if(strstr(buffer, "200 OK"))
			printf("response OK\n");	
		else
			printf("%s\n",buffer);
    }
    return nitems;
}

int main()
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    //logger_cloud("%s: Entering ... ", __FUNCTION__);

    //headers = curl_slist_append(headers, (char *) &Accept);
    //headers = curl_slist_append(headers, CLOUD_ACCEPT);
    //headers = curl_slist_append(headers, CLOUD_AUTHORIZE);

    //logger_cloud("DEBUG: %s", p_data_field);

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
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, POST_DATA);
        
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
