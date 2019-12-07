#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "sys_msg.h"

// Take the read form GPIO input 21
// If 1 Device get input from AC power source
// If 0 Device run on Battery
int wiringPiSetup();
int digitalRead();

#define PIN21	29  // RASPI PIN21 is WIRINGPI PIN29
int get_power_source()
{
    int Pin21 = -1;
    logging(DBG_INFO, "%s: Entering ...\n", __FUNCTION__);

    /* Init GPIO ports */
    if( 0 > wiringPiSetup())
	    logging(DBG_ERROR,"Failed to init GPIO lib\n");
    
	Pin21 = digitalRead(PIN21);

	if(Pin21 == 1)
	{
		logging(1,"Device running on AC power\n");
		return 1;
	}
	else
	{
		logging(1,"Device running on Battery\n");
		return 0;
	}
}


	
