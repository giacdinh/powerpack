#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>


#define SERIAL_DEV	"/dev/ttyS0"
#define AT_CMD		"AT\r\n"
#define AT_FPLMN	"AT+CRSM=176,28539,0,0,12\r\n"
#define AT_CLRFPLMN	"AT+crsm=214,28539,0,0,12,\"FFFFFFFFFFFFFFFFFFFFFFFF\"\r\r"
#define CLEAN_FPLMN	"FFFFFFFFFFFFFFFFFFFFFFFF"

int set_serial(int fd, int speed)
{
    struct termios tty;

    if (tcgetattr(fd, &tty) < 0) {
        //logging(DBG_ERROR, "Error from tcgetattr: %s\n", strerror(errno));
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
        //logging(DBG_ERROR, "Error from tcsetattr: %s\n", strerror(errno));
        printf("Error from tcsetattr: %s\n", strerror(errno));
        return -1;
    }
    return 0;
}


int main()
{
	char readbuf[128], FPLMN[32];
	int rbyte=0, fd=-1 ;
    if(access(SERIAL_DEV, 0) != 0)
    {
        //logging(DBG_ERROR,"GPS device not existed: %s\n", SERIAL_DEV);
        printf("GPS device not existed: %s\n", SERIAL_DEV);
        return -2;
    }

    fd = open(SERIAL_DEV, O_RDWR);
    if( fd < 0)
    {
        //logging(DBG_ERROR,"Serial port open failed: %s\n", SERIAL_DEV);
        printf("Serial port open failed: %s\n", SERIAL_DEV);
        return -1;
    }

    set_serial(fd, B9600);
	tcflush(fd,TCIOFLUSH);
	bzero(&readbuf[0], 128);

	write(fd, AT_CMD, strlen(AT_CMD));
	sleep(1);
	rbyte = read(fd, &readbuf[0], 128);

	// try FPLMN
	write(fd, AT_FPLMN, strlen(AT_FPLMN));
	sleep(1);
	rbyte = read(fd, &readbuf[0], 128);

	char *ptemp;
	ptemp = strstr(&readbuf[0],"\"");
	ptemp++;
	sscanf(ptemp, "%[^\"]s", &FPLMN[0]);

	printf("FPLMN: %s\n", &FPLMN[0]);
	if(0 == strncmp(CLEAN_FPLMN, &FPLMN[0], 24))
	{
		printf("FPLMN is clean\n");
	}
	else
	{
		printf("FPLMN is not clean\n");
		// try to clear FPLMN
		write(fd, AT_CLRFPLMN, strlen(AT_CLRFPLMN));
		sleep(2);
		// Try to read back FPLMN
		write(fd, AT_FPLMN, strlen(AT_FPLMN));
		sleep(1);
		bzero(&readbuf[0], 128);
		rbyte = read(fd, &readbuf[0], 128);
		printf("%s\n", &readbuf[0]);
	}
	close(fd);
	return 1;
}
