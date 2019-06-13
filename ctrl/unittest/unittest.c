#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
#include "common.h"

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

int main()
{
	int fd, i;
	char read_buf[1024];
	int readbyte;
	char *src,*dest;
	char GPGGA[128];
	NMEA_RMC_T nmea_rmc;

	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		printf("Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return -1;
	}

	set_serial(fd, B4800);

printf("set serial ok\n");

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
			if(temp = strstr((char *) &single_sentence, "$GPRMC"))
			{
				NMEA_RMC_T nmea_rmc;
				sscanf((char *) &single_sentence,"%15[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%3[^,],%15[^,],%15[^,],%15[^,]",
					nmea_rmc.gpstype, nmea_rmc.gpstime, nmea_rmc.gpswarn, nmea_rmc.gpslat, nmea_rmc.gpslatpos,
					nmea_rmc.gpslong, nmea_rmc.gpslongpos, nmea_rmc.gpsspeed, nmea_rmc.gpscourse, nmea_rmc.gpsdate);

					float declat, declong;
					declat = strtof(nmea_rmc.gpslat,NULL);
					declong = strtof(nmea_rmc.gpslong,NULL);

					float rlat, rlong;
					rlat = gpsconvert(declat);
					rlong = gpsconvert(declong);
					printf("time: %s %f, -%f\n",nmea_rmc.gpstime, rlat, rlong);
			}
			
			/* reset space for the next sentence */
			temp = (char *) &single_sentence;
			bzero(temp,128);
			usleep(5000); // Sleep 5 mils 
		}
		else
		{
			temp++;
			if((temp - single_sentence) > 100)
			{	// rewind
				temp = (char *) &single_sentence;
				bzero(temp,128);
			}
		}
	}
}

float gpsconvert(float value)
{
    int intvalue;
    float fvalue;

    intvalue = value/100.0;
	fvalue = intvalue + (float) (value - intvalue*100)/60.0;
    return fvalue;
}



