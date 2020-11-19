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
#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>

// Take the read form GPIO input 21
// If 1 Device get input from AC power source
// If 0 Device run on Battery
int wiringPiSetup();
int digitalRead();

#define PIN21	29  // RASPI PIN21 is WIRINGPI PIN29
#define PIN16	27  // RASPI PIN16 is WIRINGPI PIN27
int main()
{
    int Pin21 = -1;
    printf("%s: Entering ...\n", __FUNCTION__);

    /* Init GPIO ports */
    if( 0 > wiringPiSetup())
	    printf("Failed to init GPIO lib\n");
    
	Pin21 = digitalRead(PIN21);

	if(Pin21 == 1)
	{
		printf("Device running on AC power\n");
		return 1;
	}
	else
	{
		printf("Device running on Battery\n");
		return 0;
	}
	return -1;
}

	
