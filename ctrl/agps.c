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
#include "common.h"
#include "ctrl_common.h"
#include "sys_msg.h"

int set_gps_epo()
{
	char *atCMD[18] =  {"AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n",
					"AT+SAPBR=3,1,\"APN\",\"HOLOGRAM\"\r\n",	
					"AT+SAPBR=1,1\r\n",
					"AT+SAPBR=2,1\r\n",
					"AT+CNTPCID=1\r\n",
					"AT+CNTP=\"202.112.29.82\"\r\n",
					"AT+CNTP?\r\n",
					"AT+CNTP\r\n",
					"AT+CCLK?\r\n",
					"AT+CGNSSAV=3,3\r\n",
					"AT+HTTPINIT\r\n",
					"AT+HTTPPARA=\"CID\",1\r\n",
					"AT+HTTPPARA=\"URL\",\"http://wepodownload.mediatek.com/EPO_GPS_3_1.DAT\"\r\n",
					"AT+HTTPACTION=0\r\n",
					"AT+HTTPTERM\r\n",
					"AT+CGNSCHK=3,1\r\n",
					"AT+CGNSPWR=1\r\n",
					"AT+CGNSAID=31,1,1\r\n" };


	char *atRES[2] = {	"OK\r\n",
					"+HTTPACTION: 0,200,27648\r\n"};
    int fd, i;
	char rbuf[128];

//  logging(1,"Start init raspi hat\n");
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

	for(i = 0; i < 18; i++)
	{
		write(fd, atCMD[i], strlen(atCMD[i]));
		if(i== 11 || i == 12)
			sleep(2); //Wait about 4 second for EPO file download
		else if(i == 13)
			sleep(12);
		else
			sleep(2); // Most of command need short time for response

		// Read and check reponse
		bzero(&rbuf[0], 128);
		read(fd, &rbuf[0], 128);
		if(i == 13)
			printf("CMD#: %d long sleep -- %s",i, &rbuf[0]);

		printf("CMD#: %d -- %s",i, &rbuf[0]);
	}
}

int check_gps_epo_load_date()
{
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
	// Only load GPS EPO every 5 day and day that evenly divided by 5
	if(local->tm_mday%5 == 0)
	{
		logging(1, "Date of the month: %d\n", local->tm_mday);
		return 1;
	}
	else
		return 0;
}
