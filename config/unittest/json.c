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
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "dev_config.h"

int logging(int level, char *format, ...)
{
}

const char *process_json_data(char *ptr, char *search_key, int  *getint);

int main()
{
	int fp;
	fp = open(CONFIG_FILE, O_RDONLY);

	if(fp < 0)
	{
		printf("Open config.json failed\n");
		return -1;
	}

	char config_buf[CONFIG_FILE_SZ];
	int rbyte;
	rbyte = read(fp, &config_buf[0], CONFIG_FILE_SZ);

	if(rbyte > 0)
	{
		printf("%s\n", &config_buf[0]);
	}

	
	int value;
	char strvalue[32];
	strncpy(&strvalue[0], (char *) process_json_data(&config_buf[0], "ant1", &value), 32);
	printf("ant1: %s\n", &strvalue[0]); 
}
	

