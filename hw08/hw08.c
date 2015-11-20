#include<stdio.h>
#include<stdlib.h>
#include<sys/time.h>
#include<unistd.h>
#include<signal.h>
#include<string.h>
#include<errno.h>

static volatile sig_atomic_t timeoutFlag = 0;

static void timeoutHandler(int sig){
		
		timeoutFlag = 1;
}


int main(int argc, char *argv[]){
	char str[2048];
	char *b = str;
	
	size_t bufsize = 2048;
	size_t characters;

	int timeoutCount=0;
	struct sigaction sa;
	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = &timeoutHandler;
	sa.sa_flags = 0 ;
	sigemptyset(&sa.sa_mask);
	
	
	
	if(sigaction(SIGALRM, &sa, NULL ) != 0){
		fprintf(stderr," %s: Can not handle SIGALRM - %s",*argv, strerror(errno));
		exit(EXIT_FAILURE);
	}
	int saverr;

	while(1){
			
			alarm(3);
			printf("Enter something: ");
			characters = getline(&b, &bufsize, stdin);
			saverr = errno;
			alarm(0);
			errno = saverr;
			
			

			if(characters == -1)
			{
				if(errno == EINTR && timeoutFlag)
				{
					timeoutCount++;
					if(timeoutCount<=3){
						printf("\nTime out:%d\n",timeoutCount);
						clearerr(stdin);
						timeoutFlag = 0;
						continue;
					}
					else
					{
						exit(EXIT_SUCCESS);
					}
				}
				else if(errno == EINTR && !timeoutFlag){
					fprintf(stderr," %s: Signal other than SIGALRM received - %s. Exiting Program.\n",*argv, strerror(errno));
					exit(EXIT_FAILURE);
				}
				else{
					fprintf(stderr," %s: Error in getline - %s",*argv, strerror(errno));
					exit(EXIT_FAILURE);
				}
			}
			else{
				printf("Input[%d]: %s\n", (characters-1), str);
				continue;
			}
	}	
	return 0;
}