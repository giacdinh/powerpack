#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>


extern int set_gps_epo();
int logging(int level, char *logstr, ...)
{
	int logbuf[128];

	bzero((char *) &logbuf[0], 128);
    va_list vl;
    va_start(vl, logstr);
    vsprintf((char *) &logbuf[0], logstr, vl);
    va_end(vl);

	printf("%s", (char *) &logbuf[0]);

}

int main()
{
	logging(1,"Entering: %s\n", __FUNCTION__);
	set_gps_epo();
}
