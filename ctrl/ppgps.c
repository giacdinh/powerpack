#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include "common.h"
#include "sys_msg.h"

float gpsconvert(float value);

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

int get_gps_info(NMEA_RMC_T *rmc)
{
	int fd, i;
	char read_buf[1024];
	int readbyte;
	char *src,*dest;
	char GPGGA[128];
	int validate_cnt = 0;

	// Check to see if device node is ready
	if(access(GPS_SERIAL_DEV, 0) != 0)
	{
		logging(DBG_ERROR,"GPS device not existed: %s\n", GPS_SERIAL_DEV);
		return -2;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		logging(DBG_ERROR,"Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}

	set_serial(fd, B4800);

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
			if(temp = strstr((char *) &single_sentence, "RMC"))
			{
				//printf("%s\n", (char *) &single_sentence);
				// Take data after about 30 NMEA sentense to make sure accurate coordination
				if(validate_cnt < 27)
				{
					validate_cnt++;
				}
				else
				{
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

					close(fd);
					return 1;
				}
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



