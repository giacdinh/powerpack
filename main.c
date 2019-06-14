#include <stdio.h>
#include <pthread.h>
#include <sys_msg.h>
#include <unistd.h>
#include <string.h>

#define APP_VERSION "1.0.1"
extern void wdog_main_task();
extern void remotem_main_task();
extern void ctrl_main_task();

int main(int argc, char **argv)
{

	pid_t wdog_pid = -1;
	pid_t remotem_pid = -1;
	pid_t ctrl_pid = -1;

    if(argc > 1)
    {
        if(!strcmp(argv[1], "-v"))
        {
			printf("Run version: %s\n", APP_VERSION);
        }
        return 0;
    }

	logging(DBG_DBG, "\n");
	logging(DBG_DBG, "\n");
	logging(DBG_DBG, "**********************************\n");
	logging(DBG_DBG, "       Application %s start ...\n", APP_VERSION);
	logging(DBG_DBG, "**********************************\n");
	// Start watch dog for log
	if(wdog_pid == -1)
	{        
		wdog_pid = fork();
        if(wdog_pid == 0) // child process
        {
            logging(DBG_INFO, "Launch watchdog task ID: %i\n", getpid());
            memcpy(argv[0], "main_watchdog         \n", 26);
            wdog_main_task();
        }
    }

	// Start remotem
	if(remotem_pid == -1)
    {
        remotem_pid = fork();
        if(remotem_pid == 0) // child process
        {
            logging(DBG_DBG, "Launch remotem task ID: %i\n", getpid());
            memcpy(argv[0], "remote_system          \n", 24);
            remotem_main_task();
        }   
    }	

	// Start controller 
	if(ctrl_pid == -1)
    {
        ctrl_pid = fork();
        if(ctrl_pid == 0) // child process
        {
            logging(DBG_DBG, "Launch controller task ID: %i\n", getpid());
            memcpy(argv[0], "controller_system      \n", 24);
            ctrl_main_task();
        }   
    }	

	while(1) {
		sleep(100); // Main task done after launch children processes
	}
}
