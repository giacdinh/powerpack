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
#include "sys_msg.h"

#define LOG_PRINT_LEVEL DBG_EVENT 

int logging(int level, char *logstr, ...)
{
	int logbuf[128];
	char logfiletime[64];
	char logfilename[64];
	char timebuf[32];
	struct tm ts;
	time_t now = time(NULL);

	if(level > LOG_PRINT_LEVEL)
		return 0;
	// Get UTC time
	strftime((char *) &timebuf[0], 17, "(%Y%m%d%H%M%S)", localtime_r(&now, &ts));

	// Set up file name
	strftime((char *) &logfiletime[0], 9, "%Y%m%d", localtime_r(&now, &ts));
	sprintf((char *) &logfilename[0], "/mnt/sysdata/log/%s_log", (char *) &logfiletime[0]);

	memset((char *) &logbuf[0], 0, 128);
	va_list vl;
	va_start(vl, logstr);
	vsprintf((char *) &logbuf[0], logstr, vl);
	va_end(vl);

	// write into logfile
	int fd;
	fd = open( (char *) &logfilename , O_RDWR | O_APPEND | O_CREAT, 0666);

	if(fd < 0)
	{
		system("echo Failed to open logfile > /mnt/sysdata/log/zmgr_log");
		return -1;
	}

	// put UTC time to logfile
	write(fd, (char *) &timebuf[0], strlen(&timebuf[0]));
	write(fd,(char *) &logbuf, strlen((char *) &logbuf));

	close(fd);
}
