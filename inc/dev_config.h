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
