#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


void help(char *fileName)
{

	printf("Usage : %s <Pattern> \n",fileName);
	return ;
}

int main(int argc,char* argv[])
{

	if(argc < 2)
	{
		help(argv[0]);
		exit(EXIT_FAILURE);
	}

	pid_t child;
	int fd[2];
	char option[strlen(argv[1]+1)];
	
	pipe(fd);
	strcpy(option,argv[1]);
	char *args[] = { "/bin/ls",0 };

	if((child=fork())==-1)
	{

		perror("fork error");
		exit(EXIT_FAILURE);
	}
	else if( child == 0 )
	{
		close(fd[0]);   
        close(1);       
        dup2(fd[1],1);
		execv("/bin/ls",args);

	}
	else
	{
		wait(&child);
		close(fd[1]);
        close(0);      
        dup2(fd[0],0);
        execlp("/bin/grep","grep",option,NULL);
	}
}
