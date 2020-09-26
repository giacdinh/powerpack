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

#ifndef __CONFIGh
#define __CONFIGh

#ifdef __cplusplus
extern "C"  {
#endif

#define CONFIG_FILE "/mnt/sysdata/config/config.json"
#define CONFIG_FILE_SZ	2048
#define MAX_CONFIG_SZ	64	

typedef enum {
	UNITID=0,
	POSTURL,
	ENDPOINT,
	HOMELAT,
	HOMELONG,
	METRIC,
	CONFIG_ENUM_UNKNOWN
} CONFIG_ENUM;

static char CONFIG_FIELD[CONFIG_ENUM_UNKNOWN][MAX_CONFIG_SZ] = {
	"unitid",
	"posturl",
	"endpoint",
	"homelat",
	"homelong",
	"metric",
};

typedef struct {
	char unitid[MAX_CONFIG_SZ];
	char posturl[MAX_CONFIG_SZ];
	char endpoint[MAX_CONFIG_SZ];
	char homelat[MAX_CONFIG_SZ];
	char homelong[MAX_CONFIG_SZ];
	int metric;
} CONFIG_T;	


#endif
