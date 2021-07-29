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
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "common.h"
#include "ctrl_common.h"
#include "sys_msg.h"

#ifdef UNIT_DEBUG
#define DBG_CTRL  1
#else
#define DBG_CTRL  10
#endif


#define GPS_EPO_FLAG	"/mnt/sysdata/data/gps_epo"
#define DONOT_LOAD_EPO		1
#define LOAD_EPO			2	
#define UPDATE_EPO_FLAG		3	
#define LREADBYTE			32

int GPS_EPO_flag(int );
int GPS_EPO_nocoord_check()
{
	int date = 0, load = -1;
	int rbyte = 0, fd = -1;
	char rbuf[LREADBYTE];
	time_t now;
	time(&now);
	struct tm *local = localtime(&now);
	// Create flag

	if(access(GPS_EPO_FLAG, 0) != 0)
	{
		logging(DBG_EVENT,"GPS EPO flag not exist. Don't load GPS EPO\n");
		return DONOT_LOAD_EPO;
	}
	else // GPS_EPO_FLAG existed load and scan for setting value
	{
		fd = open(GPS_EPO_FLAG, O_RDWR);
		if(fd < 0)
		{
			logging(DBG_ERROR,"GPS EPO flag open with error: %d\n", errno);
			return -1;
		}
		rbyte = read(fd, &rbuf[0], LREADBYTE);	
		if(rbyte > 0)
		{
			if(2 != sscanf((char *) &rbuf[0],"date:%d load:%d", &date, &load)) 
			{
				logging(DBG_ERROR,"GPS EPO scan with error: %d\n", errno);
				return -1;
			}
			printf("date:%d load:%d\n", date, load);	
			if(date != local->tm_mday)
			{
				// Load GPS EPO 
				logging(DBG_CTRL,"GPS EPO loading needed\n");
				GPS_EPO_nocoord_flag(0);
				return 1;
			}
			else if (date == local->tm_mday)
			{
				// If the same day check to see if GPS EPO already loaded
				if(load == 1)
				{
					// Load GPS EPO and turn 
					GPS_EPO_nocoord_flag(0);
					logging(DBG_CTRL,"GPS EPO loading needed\n");
					return 1;
				}
				else	// GPS EPO already loaded 
					return 0;
			}
		}
	}
}

int GPS_EPO_nocoord_flag(int status)
{
	time_t now;
	time(&now);
	struct tm *local = localtime(&now);
	int fd;
	char wbuf[32];
	fd = open(GPS_EPO_FLAG, O_RDWR|O_CREAT);
	if(fd < 0)
	{
			logging(DBG_ERROR,"GPS EPO flag create with error: %d\n", errno);
			return -1;
	}
	sprintf((char *) &wbuf[0], "date:%d load:%d\n",local->tm_mday, status);
	write(fd, (char *) &wbuf[0], strlen((char *) &wbuf[0]));

	close(fd);	
	return 0;
}

int remove_GPS_EPO_nocoord_flag()
{
	logging(DBG_CTRL, "Enter: %s\n", __FUNCTION__);
	remove(GPS_EPO_FLAG);
	return 0;
}
