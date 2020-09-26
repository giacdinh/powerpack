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


#ifndef USE_RASPI_HAT
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
		return GPS_NO_DEV;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		logging(DBG_ERROR,"Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return GPS_NO_PORT;
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
					return GPS_DATA;
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

#else
int init_raspi_hat_gps()
{
	int fd, i;
    char *cmd[5] = {"AT+CGNSPWR=1\r\n", "AT+CGNSSEQ=\"RMC\"\r\n",
            "AT+CGNSINF\r\n", "AT+CGNSURC=2\r\n","AT+CGNSTST=1\r\n" };

//	logging(1,"Start init raspi hat\n");
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

	set_serial(fd, B115200);

	// Send AT command to turn on GPS on the HAT
    for(i=0; i < 5; i++)
    {
        write(fd, cmd[i], strlen(cmd[i]));
        sleep(1);
    }
	return 1;
//	logging(1,"Done init raspi hat\n");
}


int get_gps_info(NMEA_RMC_T *rmc)
{
	int fd, i;
	char read_buf[1024];
	int readbyte;
	char *src,*dest;
	int validate_cnt = 0;
	NMEA_GGA_T gga;
	int satnum = 0, no_signal = 0;

	// Check to see if device node is ready
	if(access(GPS_SERIAL_DEV, 0) != 0)
	{
		logging(DBG_ERROR,"GPS device not existed: %s\n", GPS_SERIAL_DEV);
		return GPS_NO_DEV;
	}
		
	fd = open(GPS_SERIAL_DEV, O_RDWR);
	if( fd < 0)
	{
		logging(DBG_ERROR,"Serial port open failed: %s\n", GPS_SERIAL_DEV);
		return GPS_NO_PORT;
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
			if(temp = strstr((char *) &single_sentence, "GGA"))
			{
				sscanf((char *) &single_sentence,"%15[^,],%15[^,],%15[^,],%3[^,],%15[^,],%3[^,],%3[^,],%3[^,],%7[^,],%15[^,],%3[^,]",
			        gga.gpstype, gga.gpstime, gga.gpslat ,gga.gpslatpos,
			        gga.gpslong, gga.gpslongpos, gga.gpsfix, gga.gpssat,
			        gga.gpshoz, gga.gpsalt, gga.gpsaltM);
				satnum = strtol(gga.gpssat, NULL, 10);
			}

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

				if(satnum > 3)
				    validate_cnt++;
				
				if(rmc->rlat == 0 || rmc->rlong == 0 || satnum < 4 || validate_cnt < 15)
				{
					if(no_signal++ > 120) // hang around ~20 min for TTFF (time to first fix)
						return GPS_NO_SAT;
					else
						continue;
				}

				close(fd);
				return GPS_DATA;
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
#endif // IFNDEF USE_RASPI_HAT


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



