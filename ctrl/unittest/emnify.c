#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <curl/curl.h>

#define CLOUD_ACCEPT        "Accept: application/json"
#define DEFAULT_URL			"http://detracking.tech/postdata.php/postdata?"
#define POST_DATA       "uid=DE000009&coord=%s&fwv=1.0.5&temp=35"
#define POSTURL 1
#define ENDPOINT 2

typedef struct {
    char gpstype[16];
    char gpstime[16]; 
    char gpswarn[4];  // A=OK, V=warning
    char gpslat[16];
    char gpslatpos[4];
    char gpslong[16];
    char gpslongpos[4];
    char gpsspeed[16];
    char gpscourse[16];
    char gpsdate[16];
    float rlat;
    float rlong;
} NMEA_RMC_T;

#define GPS_SERIAL_DEV "/dev/ttyS0"

int main()
{
	NMEA_RMC_T rmc;
	char coord[128];
	static int ping_cnt=0;

    while(1) 
	{

		if(0 == test_hat_power())
		{
			printf("Turn on HAT\n");
			system("sudo python /usr/local/bin/GSM_PWRKEY.py");
			sleep(10);
		}
		else
			printf("HAT port may be on, don't turn it on\n");
		
		// Wait for 30 second for it to be ready to use
		init_raspi_hat_gps();

		// Clean up coordinate structure holder before each use
		bzero((void *) &rmc, sizeof(NMEA_RMC_T));

		get_gps_info(&rmc);
		if(-1 == coord_validate(&rmc))
		{
			printf("Coordinate invalid. Sleep for a minute.\n");
		}

printf("On cell\n");
		// get core temperature 
		system("sudo /opt/vc/bin/vcgencmd measure_temp > /mnt/sysdata/log/core_temp");
		sleep(1);
		// start cellular modem connection
		printf("Get cellular connection\n");
		system("sudo pppd call gprs-emnify > /dev/null 2>&1 &");
		sleep(20);

printf("ping host\n");
		if(-1 == ping_host())
		{
			if(ping_cnt++ > 5)
			{
				ping_cnt = 0; 
				printf("Can't connect to host. Skip this post\n");	
			}
			sleep(2);
		}
		else
		{
printf("post\n");
			bzero((void *) &coord[0], 128);
			sprintf((char *) &coord[0],"%f, %f", rmc.rlat, rmc.rlong);
			printf("coordinate: %s\n", (char *) &coord[0]);
			int postresult = 0;
			postresult = postdata((char *) &coord[0], 1, 1);
			if(postresult == -1)
			{
				sleep(30);
				printf("Try to post one more time before give up\n");
				postresult = postdata((char *) &coord[0], 1, 1);
			}
		}		

		printf("kill HAT pppd session\n");
		system ("sudo killall pppd");
		sleep(15*60);		// Sleep for 4 hours
		printf("Wakeup to report data\n");
    }
}

int coord_validate(NMEA_RMC_T *rmc)
{
	if(rmc->rlat == 0.0 || rmc->rlong == 0.0)
	{
		printf("Invalid coordinate. lat: %f long:%f\n", rmc->rlat, rmc->rlong);
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

	hostinfo = gethostbyname("www.detracking.tech");
	if(hostinfo == NULL) // Not connected???
	{
		printf("pinging host failed\n");
		return -1;
	}
    for (i = 0; hostinfo->h_addr_list[i]; ++i) {
        sock_addr.sin_addr = *((struct in_addr*) hostinfo->h_addr_list[i]);
        inet_ntop(AF_INET, &sock_addr.sin_addr, ip, sizeof(ip));
        printf("hostname: detracking.tech %s\n", ip);
    }
	return 1;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

float gpsconvert(float value);

int set_serial(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        printf("Error from tcgetattr: %s\n", strerror(errno));
        return -1;
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;         /* 8-bit characters */
    tty.c_cflag &= ~PARENB;     /* no parity bit */
    tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
    tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

    /* setup for non-canonical mode */
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    /* fetch bytes as they become available */
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 1;

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


int init_raspi_hat_gps()
{
	int fd, i;
    char *cmd[5] = {"AT+CGNSPWR=1\r\n", "AT+CGNSSEQ=\"RMC\"\r\n",
            "AT+CGNSINF\r\n", "AT+CGNSURC=2\r\n","AT+CGNSTST=1\r\n" };

	// Check to see if device node is ready
	if(access(GPS_SERIAL_DEV, 0) != 0)
	{
		printf("GPS device not existed: %s\n", GPS_SERIAL_DEV);
		return -2;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		printf("Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}

	set_serial(fd, B115200);

	// Send AT command to turn on GPS on the HAT
    for(i=0; i < 5; i++)
    {
        write(fd, cmd[i], strlen(cmd[i]));
        sleep(1);
    }
	return 1;
}


int get_gps_info(NMEA_RMC_T *rmc)
{
	int fd, i;
	char read_buf[1024];
	int readbyte;
	char *src,*dest;
	int validate_cnt = 0;
	int satnum = 0, no_signal = 0;

	// Check to see if device node is ready
	if(access(GPS_SERIAL_DEV, 0) != 0)
	{
		printf("GPS device not existed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		printf("Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}

	set_serial(fd, B115200);

	ioctl(fd, TCFLSH, 2); //Flush both of read and write

	/* Init read single NMEA string */
	char single_sentence[128], *temp;
	temp = (char *) &single_sentence;
	bzero(temp,128);

	while(1)
	{
		read(fd,temp,1);
		if(*temp == '\n')
		{
			//printf("%s", (char *) &single_sentence);	
			/* Pass to process to each sentence format */
			// Check for amount of connected satellite

			if(temp = strstr((char *) &single_sentence, "RMC"))
			{
				//printf("%s\n", (char *) &single_sentence);
				sscanf((char *) &single_sentence,"%15[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%15[^,],%15[^,]",
					rmc->gpstype, rmc->gpstime, rmc->gpswarn, rmc->gpslat, rmc->gpslatpos,
					rmc->gpslong, rmc->gpslongpos, rmc->gpsspeed, rmc->gpscourse, rmc->gpsdate);

				float declat, declong;
				declat = strtof(rmc->gpslat,NULL);
				declong = strtof(rmc->gpslong,NULL);

				float rlat, rlong;
				rmc->rlat = gpsconvert(declat);
				rmc->rlong = gpsconvert(declong);

				if(!strncmp(rmc->gpslatpos,"S",1))
					rmc->rlat *= -1;

				if(!strncmp(rmc->gpslongpos,"W",1))
					rmc->rlong *= -1;

				if(rmc->rlat == 0 || rmc->rlong == 0)
				{
					if(no_signal++ > 120) // hang around ~20 min for TTFF (time to first fix)
						return -1;
					else
						continue;
				}
				printf("Lat: %f Long: %f\n", rmc->rlat,rmc->rlong);

				close(fd);
				return 1;
			}
			
			/* reset space for the next sentence */
			temp = (char *) &single_sentence;
			bzero(temp,128);
			usleep(5000); // Sleep 5 mils 
		}
		else
			temp++;
	}
	close(fd);
	return 0;
}


float gpsconvert(float value)
{
    int intvalue;
    float fvalue;

    intvalue = value/100.0;
	fvalue = intvalue + (float) (value - intvalue*100)/60.0;
    return fvalue;
}


int test_hat_power()
{
	int fd;
	fd_set set;
	struct timeval timeout;
	int rv, rbyte;
	char buff[100];
	int len = 100;

	// Check to see if device node is ready
	if(access(GPS_SERIAL_DEV, 0) != 0)
	{
		printf("GPS device not existed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		printf("Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}

	set_serial(fd, B115200);
	ioctl(fd, TCFLSH, 2); //Flush both of read and write

	// Poking the seria port before read response
	write(fd, "AT\r\n", 4);
	sleep(1);

	FD_ZERO(&set); /* clear the set */
	FD_SET(fd, &set); /* add our file descriptor to the set */

	timeout.tv_sec = 0;
	timeout.tv_usec = 30000;

	rv = select(fd + 1, &set, NULL, NULL, &timeout);
	if(rv == -1)
	{
		/* an error accured */
		return -1;
	}
	else if(rv == 0)
	{
		// Read time out, port may be not active
	//	logging(1,"Read timeout.Port OFF\n");
		return 0;
	}
	else
	{
		rbyte = read( fd, buff, len ); /* there was data to read */
		if(rbyte > 0)
		{
			if(strstr(buff,"NORMAL POWER"))
			{
	//			logging(1,"POWER DOWN NORMAL.Port OFF\n");
				return 0;
			}
			else
			{
	//			logging(1,"Read something %s. Port ON\n", buff);
				return 1;	
			}
		}
	}	
}

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

int postdata(char *coord, int power, int boot)
{
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;
    char datafield[128], *pdata;
    static char uid[32], *puid = NULL;
    char coretemp[16];
    char version[16];
    char configURL[512];
printf("Entering ... %s\n", __FUNCTION__);
    pdata = (char *) &datafield[0];

    sprintf(pdata,POST_DATA,coord);
    printf("%s\n", pdata);
	
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */
    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Cloud_Response);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, Cloud_Header_Response);

        curl_easy_setopt(curl, CURLOPT_URL, DEFAULT_URL);
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
            printf("%s:curl_easy_perform() failed: %s\n",
            __FUNCTION__, curl_easy_strerror(res));

            curl_easy_setopt(curl, CURLOPT_URL, DEFAULT_URL);
            if(res != CURLE_OK)
                printf("%s: Repost use DEFAULT_URL curl_easy_perform() failed: %s\n",
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

/*
int get_config_url(char *url)
{
    char posturl[256], endpoint[256];
    get_str_json(POSTURL,(char *) &posturl[0]);
    get_str_json(ENDPOINT,(char *) &endpoint[0]);
    sprintf(url,"%s%s",&posturl[0],&endpoint[0]);

    return 0;
}
*/
