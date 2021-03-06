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

#define BV_FAILURE (-1)
#define BV_SUCCESS (1)

static int upload_bytes = 0;
static int FileDownload_ret = BV_FAILURE;
extern int endpoint_url;

static int FileDownload_writecallback(void *ptr, size_t size, size_t nmemb, void *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
static size_t FileDownload_Header_Response(char *buffer, size_t size, size_t nitems, void *userdata)
{
    if(strstr(buffer, "HTTP"))
    {
        if(strstr(buffer, "200 OK") || strstr(buffer, "100"))
            FileDownload_ret = BV_SUCCESS;
        else
        {
            printf("%s\n",buffer);
	    FileDownload_ret = BV_FAILURE;
	}
    }
    return nitems;
}
 
int main( )
{
    CURL *curl;
    CURLcode res;
    FILE *download;

    download = fopen("./1.0.4H.tar.gz", "wb+");
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, "www.trackingde.tech/download/1.0.4H.tar.gz");
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, FileDownload_writecallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, download);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);

        /* Now run off and do what you've been told! */ 
        res = curl_easy_perform(curl);
        /* Check for errors */ 
        if(res != CURLE_OK)
            printf("curl_easy_perform() failed: %s\n",curl_easy_strerror(res));
        /* always cleanup */ 
        curl_easy_cleanup(curl);
    }
    fclose(download);
    curl_global_cleanup();

    return 0;
}
