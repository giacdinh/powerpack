#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys_msg.h>
#include <unistd.h>
#include <string.h>
#include <sys/prctl.h>
#include "build.h"

#define APP_VERSION "1.0.3"
extern void *wdog_main_task();
extern void *remotem_main_task();
extern void *ctrl_main_task();

int main(int argc, char **argv)
{

	pid_t wdog_pid = -1;
	pid_t remotem_pid = -1;
	pid_t ctrl_pid = -1;

    if(argc > 1)
    {
        if(!strcmp(argv[1], "-v"))
        {
			printf("Run version: %s %s\n", APP_VERSION, BUILDTIME);
        }
        return 0;
    }

	sleep(5);
	// Start cell network connection to git time
	system("sudo hologram network disconnect");
	sleep(1);
	system("sudo hologram network connect");
	sleep(15); // Make sure ntp doing it job
	system("sudo hologram network disconnect");

	logging(DBG_DBG, "\n");
	logging(DBG_DBG, "\n");
	logging(DBG_DBG, "**********************************\n");
	logging(DBG_DBG, "       Application %s %s start ...\n", APP_VERSION, BUILDTIME);
	logging(DBG_DBG, "**********************************\n");
	
	// Start watch dog for log
	if(wdog_pid == -1)
	{        
		wdog_pid = fork();
        if(wdog_pid == 0) // child process
        {
            logging(DBG_INFO, "Launch watchdog task ID: %i\n", getpid());
			prctl(PR_SET_NAME,"main_app_wd");
            wdog_main_task();
        }
    }

	// Start remote
#ifndef NOTUSE
	if(remotem_pid == -1)
    {
        remotem_pid = fork();
        if(remotem_pid == 0) // child process
        {
            logging(DBG_DBG, "Launch remotem task ID: %i\n", getpid());
	    prctl(PR_SET_NAME,"main_app_rm");
            remotem_main_task();
        }   
    }	
#endif

	// Start controller 
	if(ctrl_pid == -1)
    {
        ctrl_pid = fork();
        if(ctrl_pid == 0) // child process
        {
            logging(DBG_DBG, "Launch controller task ID: %i\n", getpid());
	    prctl(PR_SET_NAME,"main_app_ctrl");
            ctrl_main_task();
        }   
    }	

	while(1) {
		sleep(100); // Main task done after launch children processes
	}
}
