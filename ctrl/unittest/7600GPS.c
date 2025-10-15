#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#define SERIAL_PORT "/dev/ttyS0"

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

int sendATcommand(int serialFD, const char* ATcommand, unsigned int wsize, unsigned int wtime) 
{
	int bytes_write;
    sleep(1);
    tcflush(serialFD,TCIOFLUSH); // Clean up serial pipe for get temp call

    bytes_write = write(serialFD,ATcommand, wsize);    // Send the AT command 
	if(bytes_write != wsize)
		printf("Error: request: % bytes, write: %d bytes\n",wsize, bytes_write);

	return 0;
}

int main()
{
    int fd = -1, bytes_read;
	char read_buf[128];
    fd = open(SERIAL_PORT, O_RDWR);
    if( fd < 0)
    {
        printf("Failed to open serial port\n");
        return -1;
    }

    if( 0 > set_serial(fd, B115200))
        printf("Failed to setup serial port\n");
	//printf("Send AT command\n");
	sendATcommand(fd, "AT+CGPS=1,1\r\n", 13, 100); 
	sleep(1);
	//printf("Start read GPS\n");	
	while(1)
	{
		sendATcommand(fd, "AT+CGPSINFO\r\n", 13, 100); 
		sleep(1);
		bytes_read = read(fd, &read_buf[0],128);
		read_buf[bytes_read] = '\0';	
		printf("%s\n", &read_buf[0]);
	}
}
