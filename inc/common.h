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

//#define GPS_SERIAL_DEV "/dev/ttyUSB0"
#define GPS_SERIAL_DEV "/dev/ttyACM0" //AMA0
int get_gps_info(NMEA_RMC_T *rmc);
int logging(int level, char *logstr, ...);
char *sys_dev_id();

#define BARK_DURATION	10
