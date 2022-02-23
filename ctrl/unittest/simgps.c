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
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
//#include "common.h"
//#include "ctrl_common.h"
//#include "sys_msg.h"

int get_sim_gps(float *lat, float *lng);

#define GPS_SERIAL_DEV "/dev/ttyS0"
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
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON)    ;
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

int main()
{
	float lat,lng;

	get_sim_gps(&lat,&lng);

	printf("calling function -- lat: %f long: %f\n", lat, lng);
}

int get_sim_gps(float *lat, float *lng)
{
	char *atCMD[10] =  {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",
					"AT+SAPBR=3,1,\"APN\",\"HOLOGRAM\"\r\n",	
					"AT+SAPBR=1,1\r\n",
					"AT+SAPBR=2,1\r\n",
					"AT+CNTPCID=1\r\n",
					"AT+CNTP=\"202.112.29.82\"\r\n",
					"AT+CNTP?\r\n",
					"AT+CNTP\r\n",
					"AT+CCLK?\r\n",
					"AT+CLBS=1,1\r\n"};
    int fd, i;
	char rbuf[128];

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

	for(i = 0; i < 10; i++)
	{
		write(fd, atCMD[i], strlen(atCMD[i]));
		if(i==2)
			sleep(3);
		else if(i == 9) 
			sleep(8); //Longer than usual sleep to make sure data available
		else
			sleep(1);
		// Read and check reponse
		bzero(&rbuf[0], 128);
		read(fd, &rbuf[0], 128);
		//printf("CMD %d return: %s", i, &rbuf[0]);

		float llat,llng;
		char temp1[64],temp2[64];
		int t1,t2;
        if(i==9)
        {
            //AT+CLBS=1,1\n+CLBS: 0,-81.198814,28.679517,550
			printf("%s\n", &rbuf[0]);
            int cnt = sscanf(&rbuf[0],"%s\n%s %d,%f,%f,%d",&temp1[0],&temp2[0],&t1,&llng,&llat,&t2);
			if(cnt != 6)
				printf("cnt:%d\n",cnt);
			else
			{
				printf("scan params: %d lat: %f long: %f\n",cnt,llat,llng);
				*lat = llat;
				*lng = llng;
			}
        }
	}
	close(fd);
}

