#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


int flag_cldexited = 0 ;
int flag_cldkilled = 0 ;
int flag_cld_dumped = 0;
int flag_cld_trapped = 0;
int flag_cld_stopped = 0;
int flag_cld_continued = 0;

void childTerminateHandler(int signal, siginfo_t* siginfo, void* context)
{
	//printf("Child was terminated \n");

	int err_no = errno;

	int sigcode = siginfo->si_code ;

	if( sigcode == CLD_EXITED )
		flag_cldexited = 1 ;
	if( sigcode == CLD_KILLED )
		flag_cldkilled = 1 ;
	if( sigcode == CLD_DUMPED )
		flag_cld_dumped = 1;
	if( sigcode == CLD_TRAPPED )
		flag_cld_trapped = 1;
	if( sigcode == CLD_STOPPED )
		flag_cld_stopped = 1;
	if( sigcode == CLD_CONTINUED )
		flag_cld_continued = 1;

	errno = err_no ;
}

void childCpuTime(pid_t child)
{
	clockid_t clockid;
	struct timespec ts;

	if(clock_getcpuclockid(child, &clockid) != 0) 
	{
        perror("clock_getcpuclockid");
        exit(EXIT_FAILURE);
    }
	if (clock_gettime(clockid, &ts) == -1) 
	{
        perror("clock_gettime");
        exit(EXIT_FAILURE);
    }

    printf("Child took: %ld.%09ld seconds\n", (long) ts.tv_sec, (long) ts.tv_nsec);
}

int main(int argc, const char *argv[])
{
	pid_t child;
	int status;	

	
	//------------set up signal handler for child termination---------------------//
	struct sigaction* newChildTerminateHandler = malloc(sizeof(struct sigaction));
	//newChildTerminateHandler->sa_handler = &childTerminateHandler;
	newChildTerminateHandler->sa_flags = SA_SIGINFO ;
	newChildTerminateHandler->sa_sigaction = childTerminateHandler;
	if( sigaction(SIGCHLD, newChildTerminateHandler, NULL) == -1)
	{
		perror("Error in setting signal handler: ");
	}

	switch( child = fork() )
	{
		case -1:
			perror("Error");
		case 0:
			while(1);
			//exit(EXIT_SUCCESS);
		default:
			printf("Child process Id: %d\n", child);
			
			while(1)
			{
				if( flag_cldexited == 1 )
				{
					flag_cldexited = 0 ;
					printf("Child Exited \n");
				}
				if( flag_cldkilled == 1 )
				{
					flag_cldkilled = 0 ;
					printf("Child abnormal termination (no core dump) \n");
					childCpuTime(child);
					exit(EXIT_SUCCESS);
				}					
				if( flag_cld_dumped == 1 )
				{	
					flag_cld_dumped = 0;
					printf("Child abnormal termination (core dump)\n");
					childCpuTime(child);
					exit(EXIT_SUCCESS);
				}
				if( flag_cld_trapped == 1 )
				{
					flag_cld_trapped = 0;
					printf("Traced child stopped\n");
				}
				if( flag_cld_stopped == 1 )
				{
					flag_cld_stopped = 0;
					printf("Child Stopped\n");
				}
				if( flag_cld_continued == 1 )
				{
					flag_cld_continued = 0;
					printf("Child continued by SIGCONT\n");
				}
				
				pause();
			}
	}

	return 0;
}