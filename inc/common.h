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

#define USE_RASPI_HAT 1
//#define USE_EMNIFY	1

typedef enum {
	GPS_DATA = 0,
	GPS_NO_PORT,
	GPS_NO_DEV,
	GPS_NO_SAT
} GPS_ERROR_ID; 

//#define GPS_SERIAL_DEV "/dev/ttyUSB0"
#ifndef USE_RASPI_HAT
#define GPS_SERIAL_DEV "/dev/ttyACM0" //AMA0
#else
#define GPS_SERIAL_DEV "/dev/ttyS0" //AMA0
#endif
int get_gps_info(NMEA_RMC_T *rmc);
int logging(int level, char *logstr, ...);
char *sys_dev_id();
int init_raspi_hat_gps();
int test_hat_power();

#define BARK_DURATION	10
#define REPORT_DELAY	4	
