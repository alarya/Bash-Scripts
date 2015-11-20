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

FILE *inputFile, *outputFile;

static volatile sig_atomic_t timeoutFlag = 0;

void print_help(const char *filename)
{
	printf("Usage: %s [Input file] [Output file]\n", filename);

	exit(EXIT_SUCCESS);
}

void cleanup()
{
	//printf("\nExecuting exit function \n");

	if( inputFile != NULL)
		fclose(inputFile);

	if( outputFile != NULL )
		fclose(outputFile);

	exit(EXIT_SUCCESS);
}

void SIGINThandler()
{
	//printf("\n Interrupt handler");
	
	exit(EXIT_SUCCESS);
}

void timeoutHandler()
{
	//printf("\ntimeout\n");
	int old_err = errno;

	timeoutFlag = 1 ;

	errno = old_err;
}

int main(int argc,const char *argv[])
{
	char inputBuff[4096];
	char outputBuff[4096];
	char* inputLine = inputBuff;
	char* outputLine = outputBuff;
	//char* inputLine = NULL;
	//char* outputLine = NULL;
	size_t bufsize = 2048;
	size_t read;
	off_t offset_val = 0 ;
	char* timeStamp;
	char timeStampBuff[4096];
	char* timeStampOutput= timeStampBuff;
	time_t timeStampEpoch;

	//---------------register exit function ------------------------//
	int i = atexit(cleanup );
	if(i != 0)
	{
		fprintf((stderr), "cannot set exit function\n");
	}
	signal(SIGINT, SIGINThandler) ;

	//---------------no arguments provided -------------------------//
	if( argc == 1 )
		print_help(argv[0]);

	const char* file1 = argv[1];
	const char* file2 = argv[2];

	printf("Input File: %s\n", file1);
	printf("Output File: %s\n", file2);

	
	//--------------open file pointers --------------------------//
	if( ( inputFile = fopen( file1, "r" ) ) == NULL )
	{			
		perror("Input file: ");
		exit(EXIT_FAILURE);
	}

	if( ( outputFile = fopen( file2, "a+" ) ) == NULL )
	{			
		perror("Output file: ");
		exit(EXIT_FAILURE);
	}

	//----change default behaviour of resuming blocking calls on signal interrupts-----//
	if( siginterrupt( SIGALRM, 1 ) == -1 )
		perror("Error: ");

	//--------------set up signal handler -------------------------//
	struct sigaction newTimeoutHandler;
	newTimeoutHandler.sa_handler = &timeoutHandler ;
	newTimeoutHandler.sa_flags = 0;
	if( sigaction(SIGALRM, &newTimeoutHandler, NULL) == -1)
	{
		perror("Error in setting signal handler: ");
	}

    
    while(1)
    {

    	alarm(3);

    	//clear outputbuffer
    	outputBuff[0] = '\0';
 
    	//printf("Enter something \n");

    	read = getline( &outputLine, &bufsize ,stdin);

    	//if read frm stdin timed out, switch to read from input file
    	if(read == -1)
    	{
    	
    		//clear buffer
    		inputBuff[0] = '\0';
    		
    		//if read from console was interrupted switch to read from input file
    		if(errno == EINTR && timeoutFlag)
    		{
    			alarm(3);

    			while((read = getline( &inputLine, &bufsize, inputFile)) != -1)
    			{

    				printf("\n%s\n", inputLine);
    			}

    			clearerr(stdin);

    			timeoutFlag = 0;
    			continue;
    		}
    	}
    	//else write the user input to output file
    	else
    	{
    		// Format the output to write
    		timeStampEpoch = time(NULL);
    		timeStamp = ctime(&timeStampEpoch);
    		timeStampBuff[0] = '\0';
    		strtok(timeStamp,"\n");
    		strcpy(timeStampOutput,timeStamp);
    		strcat(timeStampOutput,",");
    		strcat(timeStampOutput,outputLine);

    		//write to output file
    		if( fprintf( outputFile,"%s",timeStampOutput) == -1 )
    		{
    			perror("Error: ");
    		}

    		fflush(NULL);

    		//printf("\nOutputLine: %s \n",timeStampOutput);

    		//reset flag
    		timeoutFlag = 0 ;
    	}

    }
	return 0;
}