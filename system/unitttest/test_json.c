#include <stdio.h>
#include <time.h>
#include <string.h>
#include <sys_msg.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <string.h>

extern int get_str_json(int, char *);

#define POSTURL 1

int main()
{
	char url[64];

	get_str_json(POSTURL,(char *) &url[0]);
	printf("Get json string: %s\n", &url[0]);

	return 0;
}
