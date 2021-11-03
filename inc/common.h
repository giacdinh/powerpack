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

typedef struct {
    char gpstype[16];
    char gpstime[16];
    char gpswarn[4];  // A=OK, V=warning
    char gpslat[16];
    char gpslatpos[4];
    char gpslong[16];
    char gpslongpos[4];
    char gpsspeed[16];
    char gpscourse[16];
    char gpsdate[16];
    float rlat;
    float rlong;
} NMEA_RMC_T;	

typedef struct {
    char gpstype[16];
    char gpstime[16];
    char gpslat[16];
    char gpslatpos[4];
    char gpslong[16];
    char gpslongpos[4];
    char gpsfix[4]; //0=invalid,1=GPS,2=DGPS,3=PPS,4=RTK,5=float,6=estimated
    char gpssat[4]; //number of satellite
    char gpshoz[8];
    char gpsalt[16];
    char gpsaltM[4];
} NMEA_GGA_T;

typedef enum {
	GPS_DATA = 0,
	GPS_NO_PORT,
	GPS_NO_DEV,
	GPS_NO_SAT
} GPS_ERROR_ID; 

int logging(int level, char *logstr, ...);
char *sys_dev_id();
int init_raspi_hat_gps();
int test_hat_power();
unsigned long get_uptime();
int get_version(char *version);

#define BARK_DURATION	10
#define REPORT_DELAY	6		// Do 6 hours each post, 4 times per day	
#define PERIODIC_REBOOT 48		// Force system reboot every 2 days to keep software in sync
#define UDP_BC_DEV		"wlan0"
