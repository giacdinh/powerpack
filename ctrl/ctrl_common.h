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

int postdata(char *,int ,int ,int ,int );
char *unit_ID();
int get_core_temp(char *);
int get_version(char *);
int get_power_source();
int set_gpio_pin16_high();
int set_gpio_pin16_reset();
int check_gps_epo_load_date();
int set_serial(int fd, int speed);
int set_gps_epo();
int get_gps_info(NMEA_RMC_T *rmc);
int GPS_EPO_nocoord_flag(int);
int remove_GPS_EPO_nocoord_flag();
int GPS_EPO_nocoord_check();
int get_sim_gps(float *, float *);
#define BACSON_HOST_NAME "trackingde.tech"
#define POPHOST1 "google.com"
#define POPHOST2 "www.yahoo.com"
#define PING_TIME	(2)
#define GPS_SERIAL_DEV "/dev/ttyS0" //AMA0
#define EPO_FLAG_FILE "/home/bacson/EPO_FLAG"
