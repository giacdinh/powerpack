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
#include <sys/ioctl.h>
#include "common.h"
#include "sys_msg.h"

#define HAT_GPS_DEV "/dev/ttyS0"

float gpsconvert(float value);
int send_gps_AT_cmd(int fd);

int set_serial(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        logging(DBG_ERROR, "Error from tcgetattr: %s\n", strerror(errno));
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
        logging(DBG_ERROR, "Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int main()
{
	NMEA_RMC_T rmc;
	get_gps_info(&rmc);	
	printf("%f,%f\n",rmc.rlat, rmc.rlong);
}

int get_gps_info(NMEA_RMC_T *rmc)
{
	int fd, i;
	char read_buf[1024];
	int readbyte;
	char *src,*dest;
	char GPGGA[128];
	NMEA_GGA_T gga;

	// Check to see if device node is ready
	if(access(HAT_GPS_DEV, 0) != 0)
	{
		logging(DBG_ERROR,"GPS device not existed: %s\n", HAT_GPS_DEV);
		return -1;
	}
		
	fd = open(HAT_GPS_DEV, O_RDWR);
	if( fd < 0)
	{
		logging(DBG_ERROR,"Serial port open failed: %s\n", HAT_GPS_DEV);
		return -1;
	}

	set_serial(fd, B115200);
	ioctl(fd, TCFLSH, 2); //Flush both of read and write

	send_gps_AT_cmd(fd);

	/* Init read single NMEA string */
	char single_sentence[128], *temp;
	temp = (char *) &single_sentence;
	bzero(temp,128);

	while(1)
	{
		read(fd,temp,1);
		if(*temp == '\n')
		{
			printf("%s", (char *) &single_sentence);	
			/* Pass to process to each sentence format */
            if(temp = strstr((char *) &single_sentence, "GGA"))
            {
                sscanf((char *) &single_sentence,"%15[^,],%15[^,],%15[^,],%3[^,],%15[^,],%3[^,],%3[^,],%3[^,],%7[^,],%15[^,],%3[^,]",
                    gga.gpstype, gga.gpstime, gga.gpslat ,gga.gpslatpos,
                    gga.gpslong, gga.gpslongpos, gga.gpsfix, gga.gpssat,
                    gga.gpshoz, gga.gpsalt, gga.gpsaltM);

				int satnum = strtol(gga.gpssat, NULL, 10);
				printf("viewed satellite: %d\n", satnum);
            }
			if(temp = strstr((char *) &single_sentence, "RMC"))
			{
			//	printf("%s\n", (char *) &single_sentence);
				sscanf((char *) &single_sentence,"%15[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%15[^,],%15[^,]",
					rmc->gpstype, rmc->gpstime, rmc->gpswarn, rmc->gpslat, rmc->gpslatpos,
					rmc->gpslong, rmc->gpslongpos, rmc->gpsspeed, rmc->gpscourse, rmc->gpsdate);

					float declat, declong;
					declat = strtof(rmc->gpslat,NULL);
					declong = strtof(rmc->gpslong,NULL);

					float rlat, rlong;
					rmc->rlat = declat;
					rmc->rlong = declong;

					rmc->rlat = gpsconvert(declat);
					rmc->rlong = gpsconvert(declong);

					if(!strncmp(rmc->gpslatpos,"S",1))
						rmc->rlat *= -1;

					if(!strncmp(rmc->gpslongpos,"W",1))
						rmc->rlong *= -1;

                    float speed;
                    speed = strtof(rmc->gpsspeed, NULL);
printf("lat: %f long: %f speed: %f\n", rmc->rlat, rmc->rlong, speed);
                     if(speed > 1.00 || speed == 0)
                        continue;

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

int send_gps_AT_cmd(int fd)
{
	char *cmd[5] = {"AT+CGNSPWR=1\r\n", "AT+CGNSSEQ=\"RMC\"\r\n", 
			"AT+CGNSINF\r\n", "AT+CGNSURC=2\r\n","AT+CGNSTST=1\r\n" };
	char resp[10];
	int i;

printf("%s: entering\n", __FUNCTION__);
	for(i=0; i < 5; i++)
	{
		write(fd, cmd[i], strlen(cmd[i]));
		sleep(1);
		read(fd,&resp[0], 5);
		printf("%s\n", &resp[0]);
	}
}


