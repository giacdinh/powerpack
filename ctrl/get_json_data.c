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
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>
#include "dev_config.h"

#ifdef UNIT_DEBUG
#define DBG_SYS  1
#else
#define DBG_SYS  10
#endif

extern const char *process_json_data(char *ptr, char *search_key, int  *getint);

int get_str_json(int itemID, char *strdata)
{
	int fp, rbyte, i;
    char confbuf[CONFIG_FILE_SZ];

    logging(DBG_INFO,"%s %d: Entering ...\n", __FUNCTION__, __LINE__);

    fp = open(CONFIG_FILE, O_RDONLY);
    if(fp < 0)
    {
        logging(DBG_ERROR, "Open config.json failed\n");
        return -1;
    }

    rbyte = read(fp, &confbuf[0], CONFIG_FILE_SZ);

	if(rbyte > 0)
	{
		strncpy(strdata, (const char *) process_json_data((char *) &confbuf[0],CONFIG_FIELD[itemID], 0),MAX_CONFIG_SZ);
		return 0;
	}
	else
		return -1;
}

int get_int_json(int itemID, int *intdata)
{
	int fp, rbyte, i;
    char confbuf[CONFIG_FILE_SZ];

    logging(DBG_INFO,"%s %d: Entering ...\n", __FUNCTION__, __LINE__);

    fp = open(CONFIG_FILE, O_RDONLY);
    if(fp < 0)
    {
        logging(DBG_ERROR, "Open config.json failed\n");
        return -1;
    }

    rbyte = read(fp, &confbuf[0], CONFIG_FILE_SZ);

	if(rbyte > 0)
	{
		process_json_data((char *) &confbuf[0], CONFIG_FIELD[itemID], intdata);
		return 0;
	}
	else
		return -1;

}
