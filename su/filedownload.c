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
 
int get_su_file(char *filename)
{
    CURL *curl;
    CURLcode res;
    FILE *download;

    download = fopen(filename, "wb+");
    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_URL, "www.trackingde.tech/download/main_app");
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
