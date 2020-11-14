#include <stdio.h>
#include <wiringPi.h>

#define PIN16	27 
int main()
{
    int Pin26 = -1;
    printf("%s: Entering ...\n", __FUNCTION__);

    /* Init GPIO ports */
    if( 0 > wiringPiSetup())
        printf("Failed to init GPIO lib\n");
	
	pinMode(PIN16, OUTPUT);

    digitalWrite(PIN16, HIGH);
	sleep(1);

    return 1;
}

