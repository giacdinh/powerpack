#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h> 
#include <termios.h>
#include <unistd.h>
//#include "common.h"
//#include "sys_msg.h"

#define HAT_GPS_DEV "/dev/ttyS0"

int test_hat_power();

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
	int result;
	result = test_hat_power();

	if(result < 0)
		printf("ERROR\n");
	else if (result == 0)
		printf("port power on\n");
	else if (result == 1)
		printf("port power off\n");
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
	if(access(HAT_GPS_DEV, 0) != 0)
	{
		printf("GPS device not existed: %s\n", HAT_GPS_DEV);
		return -1;
	}
		
	fd = open(HAT_GPS_DEV, O_RDWR);
	if( fd < 0)
	{
		printf("Serial port open failed: %s\n", HAT_GPS_DEV);
		return -1;
	}

	set_serial(fd, B115200);

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
		printf("Read error\n");
		/* an error accured */
		return -1;
	}
	else if(rv == 0)
	{
		// Read time out, port may be not active
		printf("Read timeout. Port off\n");
		return 1;
	}
	else
	{
		rbyte = read( fd, buff, len ); /* there was data to read */
		if(rbyte > 0)
		{
			if(strstr(buff,"NORMAL POWER"))
			{
				printf("Read timeout. Port off\n");
				return 1;
			}
			else
			{
				printf("Read something. Port on\n");
				return 0;	
			}
		}
	}	
}


