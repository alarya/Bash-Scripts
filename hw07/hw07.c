#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

clock_t start, end;
double cpu_time_used;
int check = 0 ;

void mHandler(int sig){
	if(sig == SIGTSTP){
		start = clock();
		pause();
	}
	else if(sig == SIGCONT){
		end = clock();
		check = 1;
	}
}

int main (int argc, char *argv[]){
	
	if (signal(SIGTSTP, mHandler) == SIG_ERR)
	        printf("\ncan't catch SIGTSTP\n");
	if (signal(SIGCONT, mHandler) == SIG_ERR)
        	printf("\ncan't catch SIGCONT\n");

    printf("My process id is %d\n", getpid());

	pause();
	
	if(check == 1)
	{
		cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
		int num_seconds = (int)cpu_time_used;
		int num_nanoseconds = (cpu_time_used - (double)num_seconds) * 1E9;
		printf("CPU time: %d seconds %d nanoseconds \n", num_seconds, num_nanoseconds);
	}
	
	return 0;
}
